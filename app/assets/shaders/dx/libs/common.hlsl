#ifndef __LIB_COMMON_HLSL__
#define __LIB_COMMON_HLSL__

static const float4 FOG_COLOR = float4(0.75f, 0.75f, 1.00f, 1.0f);

struct Light {
    float4x4 view;
    float4x4 cascades[4];
    float4 cascadesFrontPlanes[4];
    float3 strength;
    float padding_0;
    float3 direction;
    float padding_1;
};

struct Material {
    float4 diffuseAlbedo;
    float3 fresnelR0;
    float shininess;
};

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

float2x2 inverse(float2x2 m)
{
    float d = 1.0f / (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
    float2x2 _m;
    _m[0][0] = m[1][1];
    _m[0][1] = -m[0][1];
    _m[1][0] = -m[1][0];
    _m[1][1] = m[0][0];
    return _m / d;
}

#endif // __LIB_COMMON_HLSL__