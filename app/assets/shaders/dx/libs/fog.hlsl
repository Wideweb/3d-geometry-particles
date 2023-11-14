#include "/libs/noise.hlsl"

float FogFactor(float3 posW, float3 viewPos, float time)
{
    float  fogDensity = 0.05f;
    float3 fogOrigin  = viewPos;
    float3 fogDir     = normalize(posW - fogOrigin);
    float  fogDepth   = distance(posW, fogOrigin);

    float noiseSample = Noise(posW * 0.1 + float3(0.0f, time * 0.1, 0.0f)) * 0.25 + 0.75;

    fogDepth *= lerp(noiseSample * 0.5, 1.0f, saturate((fogDepth - 100.0f) / 100.0f));

    float fogFactor = exp(-fogOrigin.y * fogDensity) * (1.0f - exp(-fogDepth * fogDir.y * fogDensity)) / fogDir.y;
          fogFactor = saturate(fogFactor);

    return fogFactor;
}