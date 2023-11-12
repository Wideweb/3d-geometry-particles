#version 330 core

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
#define DEFERRED
#define PHONG
#define NOMRAL_MAPPING

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform mat4 u_view;
uniform vec3 u_viewPos;

uniform sampler2D u_colorMap;
uniform sampler2D u_positionMap;
uniform sampler2D u_normalMap;

#ifdef PHONG
uniform sampler2D u_specularMap;
#endif

#ifdef PBR
uniform sampler2D u_metallic_roughness_ambientOcclusion_map;
#endif

/////////////////////////////////////////////////////////////
//////////////////////// VARYING ////////////////////////////
/////////////////////////////////////////////////////////////
in vec2 v_texCoord;

/////////////////////////////////////////////////////////////
/////////////////////////// OUT /////////////////////////////
/////////////////////////////////////////////////////////////
layout(location = 0) out vec4 o_fragColor;

#ifdef FOG
/////////////////////////////////////////////////////////////
//////////////////////// FOG BEGIN //////////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
vec4 fog(vec4 color, vec3 fragCameraPos);

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform int u_fog;
uniform vec3 u_fogColor;
uniform float u_density;
uniform float u_gradient;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
vec4 fog(vec4 color, float distanceToCamera) {
    if (u_fog == 0) {
        return color;
    }

    float fragVisibility = exp(-pow(distanceToCamera * u_density, u_gradient));
    fragVisibility = clamp(fragVisibility, 0.0, 1.0);
    return mix(vec4(u_fogColor, 1.0), color, fragVisibility);
}

/////////////////////////////////////////////////////////////
///////////////////////// FOG END ///////////////////////////
/////////////////////////////////////////////////////////////

#endif

#ifdef BRIGHTNESS
/////////////////////////////////////////////////////////////
///////////////////// BRIGHTNESS BEGIN //////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
void saveBrightness(vec4 color);

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform float u_threshold;

/////////////////////////////////////////////////////////////
/////////////////////////// OUT /////////////////////////////
/////////////////////////////////////////////////////////////
layout(location = 2) out vec4 o_brightColor;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
void saveBrightness(vec4 color) {
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > u_threshold) {
        o_brightColor = vec4(color.rgb, 1.0);
    } else {
        o_brightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}

/////////////////////////////////////////////////////////////
////////////////////// BRIGHTNESS END ///////////////////////
/////////////////////////////////////////////////////////////

#endif

#ifdef PHONG
/////////////////////////////////////////////////////////////
//////////////////// PHONG LIGHT BEGIN //////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
///////////////// DIRECTED LIGHT DATA BEGIN /////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
struct DirectedLight {
    int pcf;
    float biasFactor;
    float biasMin;

    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    mat4 spaceMatrix;
    // sampler2D shadowMap;
};

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
const int c_maxDirectedLightsNumber = 4;

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform int u_directedLightsNumber;
uniform DirectedLight u_directedLights[c_maxDirectedLightsNumber];

/////////////////////////////////////////////////////////////
////////////////// DIRECTED LIGHT DATA END //////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
////////////////// SPOT LIGHT DATA BEGIN ////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
struct SpotLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    //samplerCube shadowMap;
    float farPlane;
    float bias;
    float pcfDiskRadius;
    int pcfSamples;
};

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
const int c_maxSpotLights = 4;

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform int u_spotLightsNumber;
uniform SpotLight u_spotLights[c_maxSpotLights];

/////////////////////////////////////////////////////////////
/////////////////// SPOT LIGHT DATA END /////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////// PHONG MATERIAL BEGIN ////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
};

struct FragmentMaterial {
    vec3 diffuse;
    vec3 specular;
    vec3 normal;
    float shininess;
};

FragmentMaterial getFragmentMaterial(vec2 texCoord, vec3 normal);

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform Material u_material;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
FragmentMaterial getFragmentMaterial(vec2 texCoord, vec3 normal) {
    vec3 diffuse = texture(u_material.diffuse, texCoord).rgb;
    vec3 specular = texture(u_material.specular, texCoord).rgb;
    return FragmentMaterial(diffuse, specular, normal, u_material.shininess);
}

/////////////////////////////////////////////////////////////
//////////////////// PHONG MATERIAL END /////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////// DIRECTED LIGHT SHADOW BEGIN /////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
float directedLightShadowCalculation(DirectedLight light, vec3 fragPos, float bias);

/////////////////////////////////////////////////////////////
////////////////// DIRECTED LIGHT SHADOW ////////////////////
/////////////////////////////////////////////////////////////
float directedLightShadowCalculation(DirectedLight light, vec3 fragPos, float bias) {
    vec4 fragPosInLightSpace = light.spaceMatrix * vec4(fragPos, 1.0);
    vec3 coords = fragPosInLightSpace.xyz / fragPosInLightSpace.w;
    coords = coords * 0.5 + 0.5;
    float currentDepth = coords.z;

    // float closestDepth = texture(light.shadowMap, coords.xy).r;
    // float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if (currentDepth > 1.0) {
        return 0.0;
    }

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);
    for (int x = -light.pcf; x <= light.pcf; ++x) {
        for (int y = -light.pcf; y <= light.pcf; ++y) {
            float pcfDepth = texture(light.shadowMap, coords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= (light.pcf * 2 + 1) * (light.pcf * 2 + 1);

    return shadow;
}

/////////////////////////////////////////////////////////////
///////////////// DIRECTED LIGHT SHADOW END /////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//////////////// PHONG LIGHT SHADOW BEGIN ///////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
const vec3 c_sampleOffsetDirections[20] = vec3[](
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1),
    vec3(-1, 1, -1), vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0), vec3(1, 0, 1), vec3(-1, 0, 1),
    vec3(1, 0, -1), vec3(-1, 0, -1), vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1));

float spotLightShadowCalculation(SpotLight light, vec3 fragPos, float bias);

/////////////////////////////////////////////////////////////
//////////////////// SPOT LIGHT SHADOW //////////////////////
/////////////////////////////////////////////////////////////
float spotLightShadowCalculation(SpotLight light, vec3 fragPos, float bias) {
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;

    for (int i = 0; i < light.pcfSamples; ++i) {
        float closestDepth =
            texture(light.shadowMap, fragToLight + c_sampleOffsetDirections[i] * light.pcfDiskRadius).r;
        closestDepth *= light.farPlane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(light.pcfSamples);

    return shadow;
}

/////////////////////////////////////////////////////////////
////////////////// PHONG LIGHT SHADOW END ///////////////////
/////////////////////////////////////////////////////////////


#ifdef SSAO
/////////////////////////////////////////////////////////////
/////////////////////// SSAO BEGIN //////////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
float ssao();

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform sampler2D u_ssao;

/////////////////////////////////////////////////////////////
///////////////////////// VARYING ///////////////////////////
/////////////////////////////////////////////////////////////
in vec2 v_fragScreenPos;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
float ssao() {

#ifdef DEFERRED
    return texture(u_ssao, v_texCoord).r;
#else
    return texture(u_ssao, v_fragScreenPos * 0.5 + 0.5).r;
#endif

}

/////////////////////////////////////////////////////////////
///////////////////////// SSAO END //////////////////////////
/////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
vec3 spotLightCalculation(SpotLight light, FragmentMaterial material, vec3 fragPos, vec3 viewDir);

vec3 directedLightCalculation(DirectedLight light, FragmentMaterial material, vec3 fragPos, vec3 viewDir);

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
vec4 phong(FragmentMaterial material, vec3 viewPos, vec3 fragPos) {
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < u_directedLightsNumber && i < c_maxDirectedLightsNumber; i++) {
        Lo += directedLightCalculation(u_directedLights[i], material, fragPos, viewDir);
    }

    for (int i = 0; i < u_spotLightsNumber && i < c_maxSpotLights; i++) {
        Lo += spotLightCalculation(u_spotLights[i], material, fragPos, viewDir);
    }

    return vec4(Lo, 1.0);
}

/////////////////////////////////////////////////////////////
////////////////////// DIRECTED LIGHT ///////////////////////
/////////////////////////////////////////////////////////////
vec3 directedLightCalculation(DirectedLight light, FragmentMaterial material, vec3 fragPos, vec3 viewDir) {
    vec3 ambient = light.ambient;

#ifdef SSAO
    ambient *= ssao();
#endif

    vec3 lightDir = normalize(-light.direction);
    float diffuseFactor = max(dot(material.normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diffuseFactor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularFactor = pow(max(dot(material.normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * material.specular * specularFactor;

    float bias = max(light.biasFactor * (1.0 - dot(material.normal, lightDir)), light.biasMin);
    float shadow = 0.0;

#ifdef DIRECTED_LIGHT_SHADOW
    shadow = directedLightShadowCalculation(light, fragPos, bias);
#endif

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * material.diffuse;

    return lighting;
}

/////////////////////////////////////////////////////////////
/////////////////////// SPOT LIGHT //////////////////////////
/////////////////////////////////////////////////////////////
vec3 spotLightCalculation(SpotLight light, FragmentMaterial material, vec3 fragPos, vec3 viewDir) {
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * material.diffuse;
#ifdef SSAO
    ambient *= ssao();
#endif

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(material.normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(material.normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = 0.0;

#ifdef SPOT_LIGHT_SHADOW
    shadow = spotLightShadowCalculation(light, fragPos, light.bias);
#endif

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    return lighting;
}

/////////////////////////////////////////////////////////////
///////////////////// PHONG LIGHT END ///////////////////////
/////////////////////////////////////////////////////////////
#endif

#ifdef PBR
/////////////////////////////////////////////////////////////
///////////////////// PBR LIGHT BEGIN ///////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
///////////////// DIRECTED LIGHT DATA BEGIN /////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
struct DirectedLight {
    int pcf;
    float biasFactor;
    float biasMin;

    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    mat4 spaceMatrix;
    // sampler2D shadowMap;
};

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
const int c_maxDirectedLightsNumber = 4;

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform int u_directedLightsNumber;
uniform DirectedLight u_directedLights[c_maxDirectedLightsNumber];

/////////////////////////////////////////////////////////////
////////////////// DIRECTED LIGHT DATA END //////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
////////////////// SPOT LIGHT DATA BEGIN ////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
struct SpotLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    //samplerCube shadowMap;
    float farPlane;
    float bias;
    float pcfDiskRadius;
    int pcfSamples;
};

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
const int c_maxSpotLights = 4;

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform int u_spotLightsNumber;
uniform SpotLight u_spotLights[c_maxSpotLights];

/////////////////////////////////////////////////////////////
/////////////////// SPOT LIGHT DATA END /////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//////////////////// PBR MATERIAL BEGIN /////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
struct Material {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D ambientOcclusion;
};

struct FragmentMaterial {
    vec3 albedo;
    vec3 normal;
    float metallic;
    float roughness;
    float ambientOcclusion;
};

FragmentMaterial getFragmentMaterial(vec2 texCoord, vec3 normal);

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform Material u_material;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
FragmentMaterial getFragmentMaterial(vec2 texCoord, vec3 normal) {
    vec3 albedo = pow(texture(u_material.diffuse, texCoord).rgb, vec3(2.2));
    float metallic = texture(u_material.metallic, texCoord).r;
    float roughness = texture(u_material.roughness, texCoord).r;
    float ambientOcclusion = texture(u_material.ambientOcclusion, texCoord).r;
    return FragmentMaterial(albedo, normal, metallic, roughness, ambientOcclusion);
}

/////////////////////////////////////////////////////////////
///////////////////// PBR MATERIAL END //////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////// DIRECTED LIGHT SHADOW BEGIN /////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
float directedLightShadowCalculation(DirectedLight light, vec3 fragPos, float bias);

/////////////////////////////////////////////////////////////
////////////////// DIRECTED LIGHT SHADOW ////////////////////
/////////////////////////////////////////////////////////////
float directedLightShadowCalculation(DirectedLight light, vec3 fragPos, float bias) {
    vec4 fragPosInLightSpace = light.spaceMatrix * vec4(fragPos, 1.0);
    vec3 coords = fragPosInLightSpace.xyz / fragPosInLightSpace.w;
    coords = coords * 0.5 + 0.5;
    float currentDepth = coords.z;

    // float closestDepth = texture(light.shadowMap, coords.xy).r;
    // float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if (currentDepth > 1.0) {
        return 0.0;
    }

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);
    for (int x = -light.pcf; x <= light.pcf; ++x) {
        for (int y = -light.pcf; y <= light.pcf; ++y) {
            float pcfDepth = texture(light.shadowMap, coords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= (light.pcf * 2 + 1) * (light.pcf * 2 + 1);

    return shadow;
}

/////////////////////////////////////////////////////////////
///////////////// DIRECTED LIGHT SHADOW END /////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//////////////// PHONG LIGHT SHADOW BEGIN ///////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
const vec3 c_sampleOffsetDirections[20] = vec3[](
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1),
    vec3(-1, 1, -1), vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0), vec3(1, 0, 1), vec3(-1, 0, 1),
    vec3(1, 0, -1), vec3(-1, 0, -1), vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1));

float spotLightShadowCalculation(SpotLight light, vec3 fragPos, float bias);

/////////////////////////////////////////////////////////////
//////////////////// SPOT LIGHT SHADOW //////////////////////
/////////////////////////////////////////////////////////////
float spotLightShadowCalculation(SpotLight light, vec3 fragPos, float bias) {
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;

    for (int i = 0; i < light.pcfSamples; ++i) {
        float closestDepth =
            texture(light.shadowMap, fragToLight + c_sampleOffsetDirections[i] * light.pcfDiskRadius).r;
        closestDepth *= light.farPlane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(light.pcfSamples);

    return shadow;
}

/////////////////////////////////////////////////////////////
////////////////// PHONG LIGHT SHADOW END ///////////////////
/////////////////////////////////////////////////////////////


#ifdef SSAO
/////////////////////////////////////////////////////////////
/////////////////////// SSAO BEGIN //////////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
float ssao();

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform sampler2D u_ssao;

/////////////////////////////////////////////////////////////
///////////////////////// VARYING ///////////////////////////
/////////////////////////////////////////////////////////////
in vec2 v_fragScreenPos;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
float ssao() {

#ifdef DEFERRED
    return texture(u_ssao, v_texCoord).r;
#else
    return texture(u_ssao, v_fragScreenPos * 0.5 + 0.5).r;
#endif

}

/////////////////////////////////////////////////////////////
///////////////////////// SSAO END //////////////////////////
/////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
vec3 directedLightEnergyBrightness(DirectedLight light, FragmentPBRMaterial material, vec3 fragPos, vec3 viewDir,
                                   vec3 surfaceReflectance);

vec3 spotLightEnergyBrightness(SpotLight light, FragmentPBRMaterial material, vec3 fragPos, vec3 viewDir,
                               vec3 surfaceReflectance);

vec3 lightEnergyBrightness(vec3 lightDir, vec3 lightColor, FragmentPBRMaterial material, vec3 fragPos, vec3 viewDir,
                           vec3 surfaceReflectance);

float distributionGGX(vec3 N, vec3 H, float roughness);

float geometrySchlickGGX(float NdotV, float roughness);

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

vec3 fresnelSchlick(float cosTheta, vec3 F0);

/////////////////////////////////////////////////////////////
//////////////////////// DEFINES ////////////////////////////
/////////////////////////////////////////////////////////////
const float PI = 3.14159265359;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
vec4 pbr(FragmentMaterial material, vec3 viewPos, vec3 fragPos) {
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 surfaceReflectance = vec3(0.04);
    surfaceReflectance = mix(surfaceReflectance, material.albedo, material.metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < u_directedLightsNumber; i++) {
        Lo += directedLightEnergyBrightness(u_directedLights[i], material, fragPos, viewDir, surfaceReflectance);
    }

    for (int i = 0; i < u_spotLightsNumber; i++) {
        Lo += spotLightEnergyBrightness(u_spotLights[i], material, fragPos, viewDir, surfaceReflectance);
    }

    vec3 ambient = vec3(0.03) * material.albedo * material.ambientOcclusion;

#ifdef SSAO
    ambient *= ssao();
#endif

    return vec4(ambient + Lo, 1.0);
}

/////////////////////////////////////////////////////////////
////////////////////// DIRECTED LIGHT ///////////////////////
/////////////////////////////////////////////////////////////
vec3 directedLightEnergyBrightness(DirectedLight light, FragmentPBRMaterial material, vec3 fragPos, vec3 viewDir,
                                   vec3 surfaceReflectance) {
    vec3 lightDir = normalize(-light.direction);
    float bias = max(light.biasFactor * (1.0 - dot(material.normal, lightDir)), light.biasMin);
    float shadow = 0.0;

#ifdef DIRECTED_LIGHT_SHADOW
    shadow = directedLightShadowCalculation(light, fragPos, bias);
#endif

    vec3 LE = lightEnergyBrightness(lightDir, light.diffuse, material, fragPos, viewDir, surfaceReflectance);
    return (1.0 - shadow) * LE;
}

/////////////////////////////////////////////////////////////
/////////////////////// SPOT LIGHT //////////////////////////
/////////////////////////////////////////////////////////////
vec3 spotLightEnergyBrightness(SpotLight light, FragmentPBRMaterial material, vec3 fragPos, vec3 viewDir,
                               vec3 surfaceReflectance) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float shadow = 0.0;

#ifdef SPOT_LIGHT_SHADOW
    shadow = spotLightShadowCalculation(light, fragPos, light.bias);
#endif

    vec3 LE = lightEnergyBrightness(lightDir, light.diffuse, material, fragPos, viewDir, surfaceReflectance);
    return (1.0 - shadow) * LE * attenuation;
}

vec3 LightEnergyBrightness(vec3 lightDir, vec3 lightColor, FragmentPBRMaterial material, vec3 fragPos, vec3 viewDir,
                           vec3 surfaceReflectance) {
    vec3 halfwayDir = normalize(viewDir + lightDir);
    vec3 radiance = lightColor;

    // Cook-Torrance BRDF
    float NDF = distributionGGX(material.normal, halfwayDir, material.roughness);
    float G = geometrySmith(material.normal, viewDir, lightDir, material.roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), surfaceReflectance);

    vec3 kSpecular = F;
    vec3 kDiffuse = vec3(1.0) - kSpecular;
    kDiffuse *= 1.0 - material.metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(material.normal, viewDir), 0.0) * max(dot(material.normal, lightDir), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    float normalDotLightDir = max(dot(material.normal, lightDir), 0.0);
    vec3 Lo = (kDiffuse * material.albedo / PI + specular) * radiance * normalDotLightDir;

    return Lo;
}

// Уравнение Френеля
// Описывает отношение отраженного и преломленного света,
// которое зависит от угла, под который мы смотрим на поверхность
vec3 fresnelSchlick(float cosTheta, vec3 F0) { return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); }

// Функция нормального распределения.
// Аппроксимация общего выравнивания микрограней по медианному вектору H
// с учетом параметра шероховатости
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Функция геометрии
// Статистически аппроксимирует относительную площадь поверхности,
// где ее микроскопические неровности перекрывают друг друга
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

/////////////////////////////////////////////////////////////
////////////////////// PBR LIGHT END ////////////////////////
/////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
void main() {
    vec3 fragColor = texture(u_colorMap, v_texCoord).rgb;
    vec3 fragPos = texture(u_positionMap, v_texCoord).rgb;
    vec3 fragNormal = texture(u_normalMap, v_texCoord).rgb;

#ifdef PHONG
    vec4 ss = texture(u_specularMap, v_texCoord);

    FragmentMaterial material = FragmentMaterial(fragColor, ss.rgb, fragNormal, ss.a);

    o_fragColor = phong(material, u_viewPos, fragPos);
#endif

#ifdef PBR
    vec3 mra = texture(u_metallic_roughness_ambientOcclusion_map).rgb;

    FragmentMaterial material = FragmentMaterial(fragColor, fragNormal, mra.r, mra.g, mra.b);

    o_fragColor = pbr(material, u_viewPos, fragPos);
#endif

#ifdef FOG
    vec4 fragCameraPos = u_view * vec4(fragPos, 1.0);
    o_fragColor = fog(o_fragColor, length(fragCameraPos));
#endif

#ifdef BRIGHTNESS
    saveBrightness(o_fragColor);
#endif
}