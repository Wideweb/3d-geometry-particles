// cbuffer cbPerObject : register(b0)
// {
// 	float4x4 gWorld; 
// };

struct VertexIn
{
	float3 PosL      : POSITION;
    float3 Normal    : NORMAL;
    float2 TexCoord  : TEXCOORD;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float4 Color     : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
    // float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosH = float4(vin.PosL, 1.0f);// mul(posW, gViewProj);
    vout.PosH.z = 0.5;
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}