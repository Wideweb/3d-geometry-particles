#include "light-lib.hlsl"

cbuffer cbCommon : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 viewPos;
    float4 ambientLight;
    Light light;
};

cbuffer cbObject : register(b1)
{
	float4x4 model;
};

TextureCube cubeMap : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

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
    float4 PosH     : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    float4x4 fixedView = view;
    fixedView[0][3] = 0;
    fixedView[1][3] = 0;
    fixedView[2][3] = 0;
    fixedView[3][0] = 0;
    fixedView[3][1] = 0;
    fixedView[3][2] = 0;
    fixedView[3][3] = 1;

    float4 pos = mul(float4(vin.PosL, 1.0), model);
    pos = mul(pos, fixedView);
    pos = mul(pos, projection);
    
    vout.PosH = pos.xyww;
    vout.TexCoord = vin.PosL;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return cubeMap.Sample(gsamLinearWrap, pin.TexCoord);
}