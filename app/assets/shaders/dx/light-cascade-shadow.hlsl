#include "libs/common.hlsl"
#include "libs/light.hlsl"
#include "libs/shadows.hlsl"
#include "libs/fog.hlsl"

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
    float3 CascadeBlend : POSITION2;
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

    vout.CascadeBlend[0] = dot(posW, light.cascadesFrontPlanes[1]);
    vout.CascadeBlend[1] = dot(posW, light.cascadesFrontPlanes[2]);
    vout.CascadeBlend[2] = dot(posW, light.cascadesFrontPlanes[3]);
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 normal = normalize(pin.NormalW);

    float3 toEye = normalize(viewPos - pin.PosW);
    float4 ambient = ambientLight * diffuseAlbedo;

    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };

    float bias = max(0.01 * (1.0 - dot(normal, light.direction)), 0.005);
    float3 shadowFactor = CalcShadowFactorWithBlending(pin.PosW, pin.PosV, light, shadowMap, pin.CascadeBlend, bias);
    
    float4 directLight = ComputeLighting(light, mat, pin.PosW, normal, toEye, shadowFactor);

    float4 litColor = ambient + directLight;
    litColor.a = diffuseAlbedo.a;

    return lerp(litColor, FOG_COLOR, FogFactor(pin.PosW, viewPos, time));

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// DEBUG ///////////////////////////////////
    // return DebugCascadeBlending(pin.CascadeBlend);
    ////////////////////////////////// DEBUG ///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
}