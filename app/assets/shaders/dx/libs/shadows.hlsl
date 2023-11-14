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

float CalcShadowFactorWithPCF(float3 posW, Light light, float2 coords, float currentDepth, Texture2D shadowMap)
{
    // float4 posInLight = mul(float4(posW, 1.0f), light.view);
    // float4 posInLightDx = ddx(posInLight);
    // float4 posInLightDy = ddy(posInLight);

    // float2x2 dxdy = float2x2(
    //     float2(posInLightDx.x, posInLightDy.x),
    //     float2(posInLightDx.y, posInLightDy.y)
    // );
    // float2x2 dxdyInv = inverse(dxdy);

    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);

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
        // float2 screenSpaceOffset = mul(offsets[i], dxdyInv);
        // float dz = screenSpaceOffset.x * posInLightDx.z + screenSpaceOffset.y * posInLightDy.z;

        percentLit += shadowMap.SampleCmpLevelZero(gsamShadow, coords.xy + offsets[i], currentDepth).r;
    }
    
    return percentLit / 9.0f;
}

float CalcShadowFactorWithBlending(float3 posW, float3 posV, Light light, Texture2D shadowMap[4], float3 cascadeBlend, float bias)
{
    bool beyondCascade2 = cascadeBlend.y >= 0.0f;
    bool beyondCascade3 = cascadeBlend.z >= 0.0f;

    uint layer1 = beyondCascade2 ? 2 : 0;
    uint layer2 = beyondCascade3 ? 3 : 1;

    float4 posCascade1 = mul(float4(posW, 1.0f), light.cascades[layer1]);
    float3 projCoords1 = posCascade1.xyz / posCascade1.w;

    float4 posCascade2 = mul(float4(posW, 1.0f), light.cascades[layer2]);
    float3 projCoords2 = posCascade2.xyz / posCascade2.w;
    
    float3 blend = saturate(cascadeBlend);
    float weight = beyondCascade2 ? (blend.y - blend.z) : (1.0f - blend.x);

    float shadow1 = CalcShadowFactorWithPCF(posW, light, posCascade1.xy, posCascade1.z - bias, shadowMap[layer1]);
    float shadow2 = CalcShadowFactorWithPCF(posW, light, posCascade2.xy, posCascade2.z - bias, shadowMap[layer2]);

    return lerp(shadow2, shadow1, weight);
}

float4 DebugCascadeBlending(float3 cascadeBlend)
{
    float stroke = 0.01f;
    const float4 colors[4] =
    {
        float4(1.0, 0.0, 0.0, 1.0),
        float4(0.0, 1.0, 0.0, 1.0),
        float4(0.0, 0.0, 1.0, 1.0),
        float4(1.0, 0.0, 1.0, 1.0)
    };

    bool beyondCascade2 = cascadeBlend.y >= 0.0f;
    bool beyondCascade3 = cascadeBlend.z >= 0.0f;

    uint layer1 = beyondCascade2 ? 2 : 0;
    uint layer2 = beyondCascade3 ? 3 : 1;

    float3 blend = saturate(cascadeBlend);
    float weight = cascadeBlend.y >= 0.0f ? (blend.y - blend.z) : (1.0f - blend.x);

    return lerp(colors[layer2], colors[layer1], weight);
}