#ifndef __LIB_NOISE_HLSL__
#define __LIB_NOISE_HLSL__

float3 Random3(float3 p)
{
    float d1 = dot(p, float3(12.3, 32.1, 21.3));
    float d2 = dot(p, float3(45.6, 65.4, 54.6));
    float d3 = dot(p, float3(78.9, 98.7, 87.9));

    p = float3(d1, d2, d3);
    return frac(sin(p) * 14.7) * 2.0 - 1.0;
}

float Noise(float3 st)
{
    float3 i = floor(st);
    float3 f = frac(st);

    float3 u = smoothstep(0.0, 1.0, f);

    float d1 = dot(Random3(i + float3(0.0, 0.0, 0.0)), f - float3(0.0, 0.0, 0.0));
    float d2 = dot(Random3(i + float3(1.0, 0.0, 0.0)), f - float3(1.0, 0.0, 0.0));
    float d3 = dot(Random3(i + float3(0.0, 1.0, 0.0)), f - float3(0.0, 1.0, 0.0));
    float d4 = dot(Random3(i + float3(1.0, 1.0, 0.0)), f - float3(1.0, 1.0, 0.0));

    float d5 = dot(Random3(i + float3(0.0, 0.0, 1.0)), f - float3(0.0, 0.0, 1.0));
    float d6 = dot(Random3(i + float3(1.0, 0.0, 1.0)), f - float3(1.0, 0.0, 1.0));
    float d7 = dot(Random3(i + float3(0.0, 1.0, 1.0)), f - float3(0.0, 1.0, 1.0));
    float d8 = dot(Random3(i + float3(1.0, 1.0, 1.0)), f - float3(1.0, 1.0, 1.0));

    float m1 = lerp(lerp(d1, d2, u.x), lerp(d3, d4, u.x), u.y);
    float m2 = lerp(lerp(d5, d6, u.x), lerp(d7, d8, u.x), u.y);

    return lerp(m1, m2, u.z);
}

float FBM(float3 st)
{ 
    const uint octaves = 5;
    float lacunarity = 2.0;
    float gain = 0.5;

    float amplitude = 1.0;
    float frequency = 1.0;

    float y = 0.0;
    for (uint i = 0; i < octaves; i++)
    {
        y += amplitude * Noise(frequency * st);
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return y;
}

float Turbulence(float3 st)
{ 
    const int octaves = 5;
    float lacunarity = 2.0;
    float gain = 0.5;

    float amplitude = 1.0;
    float frequency = 1.0;

    float y = 0.0;
    for (int i = 0; i < octaves; i++)
    {
        y += amplitude * abs(Noise(frequency * st));
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return y;
}

float Ridge(float h)
{
    h = abs(h);
    h = 1.0 - h;
    h = h * h;
    return h;
}

float Ridget(float3 st) { 
    const int octaves = 5;
    float lacunarity = 2.0;
    float gain = 0.5;

    float amplitude = 1.0;
    float frequency = 1.0;

    float y = 0.0;
    float normalization = 0.0;
    for (int i = 0; i < octaves; i++)
    {
        y += amplitude * Ridge(Noise(frequency * st));
        normalization += amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return y / normalization;
}

#endif // __LIB_NOISE_HLSL__