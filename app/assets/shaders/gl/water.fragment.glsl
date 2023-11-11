#version 330 core

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
float noise(vec3 st);
float turbulence(vec3 st);
vec3 random3(vec3 st);

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform sampler2D u_reflection;
uniform sampler2D u_refraction;
uniform float u_time;
uniform vec3 u_viewPos;

/////////////////////////////////////////////////////////////
//////////////////////// VARYING ////////////////////////////
/////////////////////////////////////////////////////////////
in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_fragPos;

/////////////////////////////////////////////////////////////
/////////////////////////// OUT /////////////////////////////
/////////////////////////////////////////////////////////////
layout(location = 0) out vec4 o_fragColor;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
void main() {
    float noiseValue2 = turbulence(vec3(v_texCoord * 5.0, u_time * 0.1));
    float t = (v_texCoord.x + noiseValue2) * 6.28 * 0.5;
    float result = sin(t) * 0.5 + 0.5;
    float r = result * 0.15 + sin(u_time) * 0.1;

    float depth = smoothstep(0.2 + r, 0.3 + r, v_texCoord.x);
    
    float wave = smoothstep(0.1 + r, 0.18 + r, v_texCoord.x);
    wave *= (1.0 - smoothstep(0.2 + r, 0.3 + r, v_texCoord.x));
    wave *= (step(0.1 + r, v_texCoord.x));


    // float r3 = result * 0.15 - fract(u_time * 0.25) * 0.1 + 0.2;
    // float wave2 = smoothstep(0.1 + r3, 0.18 + r3, v_texCoord.x);
    // wave2 *= (1.0 - smoothstep(0.2 + r3, 0.3 + r3, v_texCoord.x));
    // wave2 *= (step(0.1 + r, v_texCoord.x));

    // wave += wave2;


    float noiseValue = noise(vec3(v_texCoord * 40.0, u_time * 2.0)) * 0.2;
    vec2 reflectCoord = v_texCoord + noiseValue * depth;
    vec2 refractCoord = v_texCoord * (1.0 + v_texCoord.x * 2.0) + noiseValue * 0.25 * depth;

    vec3 reflection = texture(u_reflection, reflectCoord).rgb;
    vec3 refraction = texture(u_refraction, refractCoord).rgb;

    vec3 viewDir = normalize(u_viewPos - v_fragPos);
    float reflectFactor = 1.0 - dot(viewDir, v_normal) * 0.5;
    

    vec3 color = mix(refraction, reflection, reflectFactor * depth);
    color = mix(color, vec3(0.0, 0.0, 0.2), clamp(v_texCoord.x * 0.25, 0.0, 0.5));

    color = mix(color, vec3(0.95, 0.95, 1.0), wave);


    o_fragColor = vec4(color, 1.0);
}

float turbulence(vec3 st) { 
  const int octaves = 5;
  float lacunarity = 2.0;
  float gain = 0.5;
  
  float amplitude = 1.0;
  float frequency = 1.0;
  
  float y = 0.0;
  for (int i = 0; i < octaves; i++) {
    y += amplitude * abs(noise(frequency * st));
    frequency *= lacunarity;
    amplitude *= gain;
  }
  
  return y;
}

float noise(vec3 st) {
  vec3 i = floor(st);
  vec3 f = fract(st);

  vec3 u = smoothstep(0.0, 1.0, f);
  
  float d1 = dot(random3(i + vec3(0.0, 0.0, 0.0)), f - vec3(0.0, 0.0, 0.0));
  float d2 = dot(random3(i + vec3(1.0, 0.0, 0.0)), f - vec3(1.0, 0.0, 0.0));
  float d3 = dot(random3(i + vec3(0.0, 1.0, 0.0)), f - vec3(0.0, 1.0, 0.0));
  float d4 = dot(random3(i + vec3(1.0, 1.0, 0.0)), f - vec3(1.0, 1.0, 0.0));
  
  float d5 = dot(random3(i + vec3(0.0, 0.0, 1.0)), f - vec3(0.0, 0.0, 1.0));
  float d6 = dot(random3(i + vec3(1.0, 0.0, 1.0)), f - vec3(1.0, 0.0, 1.0));
  float d7 = dot(random3(i + vec3(0.0, 1.0, 1.0)), f - vec3(0.0, 1.0, 1.0));
  float d8 = dot(random3(i + vec3(1.0, 1.0, 1.0)), f - vec3(1.0, 1.0, 1.0));
  
  float m1 = mix(mix(d1, d2, u.x), mix(d3, d4, u.x), u.y);
  float m2 = mix(mix(d5, d6, u.x), mix(d7, d8, u.x), u.y);
  
  float result = mix(m1, m2, u.z);
  result = result * 0.5 + 0.5;
  
  return result;
}

vec3 random3(vec3 st) {
  float d1 = dot(st, vec3(12.3, 32.1, 21.3));
  float d2 = dot(st, vec3(45.6, 65.4, 54.6));
  float d3 = dot(st, vec3(78.9, 98.7, 87.9));
  
  st = vec3(d1, d2, d3);
  return fract(sin(st) * 14.7) * 2.0 - 1.0;
}