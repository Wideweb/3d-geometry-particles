#pragma once

#include "GfxEffect.hpp"

#include <memory>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

class CascadeShadowEffect : public GfxEffect {
public:
    void bind() override;
    void update(GfxEffect::RenderCommonData& commonData) override;
    void draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) override;

private:
    struct RenderItemData {
        glm::mat4 model;
    };

    struct DepthRenderCommonData {
        glm::mat4 viewProj;
    };

    struct DepthMap {
        std::shared_ptr<Engine::CrossPlatformDepthStencilTexture> texture;
        std::shared_ptr<Engine::CrossPlatformFramebuffer> framebuffer;
        std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> renderData;
    };

    void initDepthPass();
    void initLightPass();
    void initDebugPass();

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_LightShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_LightRenderPass;

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_DepthShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_DepthRenderPass;
    std::array<DepthMap, 4> m_DepthMaps;

    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_TerrainRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_TerrainMaterialRenderData;

    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_InstancesRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_InstanceMaterialRenderData;

    // DEBUG
    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_ScreenTextureShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_ScreenTextureRenderPass;
    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_ScreenRenderData;
    // DEBUG

    std::array<float, 8> m_CascadeDistances = {
        0.1f, 16.1f,
        14.0f, 46.0f,
        42.0f, 74.0f,
        70.0f, 102.0f
    };
};