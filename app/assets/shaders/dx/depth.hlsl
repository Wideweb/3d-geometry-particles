cbuffer cbCommon : register(b0)
{
    float4x4 viewProj;
};

cbuffer cbObject : register(b1)
{
	float4x4 model;
};

struct VertexIn
{
	float3 PosL    : POSITION;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    float4 posL = float4(vin.PosL, 1.0);
    float4 posW = mul(posL, model);
    vout.PosH   = mul(posW, viewProj);
	
    return vout;
}