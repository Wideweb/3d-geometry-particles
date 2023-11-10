#include "light-lib.hlsl"

cbuffer cbCommon : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 viewPos;
    float time;
    float4 ambientLight;
    Light light;
};

cbuffer cbObject : register(b1)
{
	float4x4 model;
};

cbuffer cbMaterial : register(b2)
{
	float4 diffuseAlbedo;
    float3 fresnelR0;
    float  roughness;
};

Texture2D shadowMap[4] : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

struct VertexIn
{
	float3 PosL      : POSITION;
    float3 NormalL   : NORMAL;
    float2 TexCoord  : TEXCOORD;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float4 Color     : COLOR;
};

struct VertexOut
{
    float3 PosW         : POSITION0;
    float3 PosV         : POSITION1;
    float4 PosH         : SV_POSITION;
    float3 NormalW      : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    float4 pos = float4(vin.PosL, 1.0);
    float4 posW = mul(pos, model);
    float4 posV = mul(posW, view);
    float4 posH = mul(posV, projection);

    vout.PosW = posW.xyz;
    vout.PosV = posV.xyz;
    vout.PosH = posH;
    vout.NormalW = mul(vin.NormalL, (float3x3) model);
	
    return vout;
}

//---------------------------------------------------------------------------------------
// PCF for shadow mapping.
//---------------------------------------------------------------------------------------

float CalcShadowFactor(float3 posW, float3 posV, float bias)
{
    // Depth in NDC space.
    float depth = abs(posV.z);

    uint layer = 3;
    if (depth < 15.0f) {
        layer = 0;
    } else if (depth < 44.0f) {
        layer = 1;
    } else if (depth < 72.0f) {
        layer = 2;
    }

    float4 posC = mul(float4(posW, 1.0f), light.cascades[layer].viewProj);
    float3 projCoords = posC.xyz / posC.w;
    float currentDepth = projCoords.z;

    uint width, height, numMips;
    shadowMap[layer].GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float)width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for(int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap[layer].SampleCmpLevelZero(gsamShadow, projCoords.xy + offsets[i], currentDepth - bias).r;
    }
    
    return percentLit / 9.0f;
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 normal = normalize(pin.NormalW);

    // Vector from point being lit to eye. 
    float3 toEye = normalize(viewPos - pin.PosW);

	// Indirect lighting.
    float4 ambient = ambientLight * diffuseAlbedo;

    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };

    float bias = max(0.01 * (1.0 - dot(normal, light.direction)), 0.005);
    float3 shadowFactor = CalcShadowFactor(pin.PosW, pin.PosV, bias);
    
    float4 directLight = ComputeLighting(light, mat, pin.PosW, normal, toEye, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}