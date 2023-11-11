#include "libs/common.hlsl"

cbuffer cbObject : register(b0)
{
	float4x4 model;
};

Texture2D diffuseMap : register(t0);

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
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    vout.PosH = mul(model, float4(vin.PosL, 1.0));
    vout.TexCoord = vin.TexCoord;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return float4(diffuseMap.Sample(gsamLinearWrap, pin.TexCoord).rrr, 1.0);
}