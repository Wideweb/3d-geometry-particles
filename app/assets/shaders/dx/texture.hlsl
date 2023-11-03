cbuffer cbCommon : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

cbuffer cbObject : register(b1)
{
	float4x4 model;
};

Texture2D diffuseMap : register(t0);

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
    float4 PosW     : POSITION0;
    float4 PosV     : POSITION1;
    float4 PosH     : SV_POSITION;
    float3 NormalW  : NORMAL;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    vout.PosW = mul(float4(vin.PosL, 1.0), model);
    vout.PosV = mul(vout.PosW, view);
    vout.PosH = mul(vout.PosV, projection);
    vout.NormalW = mul(vin.NormalL, (float3x3) model);
    vout.Color = vin.Color;
    vout.TexCoord = vin.TexCoord;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return diffuseMap.Sample(gsamAnisotropicWrap, pin.TexCoord);
    // return float4(normalize(pin.NormalW), 1.0);
    // return pin.Color;
}