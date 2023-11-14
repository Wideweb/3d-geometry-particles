#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

#include "GfxEffect.hpp"

class SkyboxNoiseEffect : public GfxEffect {
public:
    void bind() override;
    void update(GfxEffect::RenderCommonData& commonData) override;
    void draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) override;

private:
    struct RenderItemData {
        glm::mat4 model;
    };

    std::shared_ptr<Engine::CrossPlatformTexture>                 m_SkyboxCubeTexture;
    std::shared_ptr<Engine::CrossPlatformShaderProgram>           m_SkyboxShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass>              m_SkyboxRenderPass;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_SkyboxRenderData;
};