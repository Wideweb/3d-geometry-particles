#include "/libs/common.hlsl"

float CalcShadowFactor(float3 posW, float3 posV, Light light, Texture2D shadowMap[4], float bias)
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

    float4 posC = mul(float4(posW, 1.0f), light.cascades[layer]);
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