#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

#include "Engine.hpp"

class GfxEffect {
public:
    struct RenderLightData {
        glm::mat4 view;
        glm::mat4 cascades[4];
        glm::vec4 cascadesFrontPlanes[4];
        glm::vec3 strength;
        float     padding_0;
        glm::vec3 direction;
        float     padding_1;
    };

    struct RenderCommonData {
        glm::mat4       view;
        glm::mat4       projection;
        glm::vec3       viewPos;
        float           time;
        glm::vec4       ambientLight;
        RenderLightData light;
    };

    struct RenderMaterialData {
        glm::vec4 diffuseAlbedo;
        glm::vec3 fresnelR0;
        float     roughness;
    };

    virtual void bind()                                                                         = 0;
    virtual void update(GfxEffect::RenderCommonData& commonData)                                = 0;
    virtual void draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) = 0;
};