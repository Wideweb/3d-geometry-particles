#ifndef __LIB_COMMON_HLSL__
#define __LIB_COMMON_HLSL__

struct Light {
    float4x4 view;
    float4x4 cascades[4];
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

#endif // __LIB_COMMON_HLSL__