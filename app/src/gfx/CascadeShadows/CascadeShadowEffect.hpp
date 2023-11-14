#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

#include "GfxEffect.hpp"

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
        std::shared_ptr<Engine::CrossPlatformDepthStencilTexture>     texture;
        std::shared_ptr<Engine::CrossPlatformFramebuffer>             framebuffer;
        std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> renderData;
    };

    void initDepthPass();
    void initLightPass();
    void initDebugPass();

    void initTerrain();
    void initInstances();

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_LightShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass>    m_LightRenderPass;

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_DepthShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass>    m_DepthRenderPass;
    std::array<DepthMap, 4>                             m_DepthMaps;

    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_TerrainRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_TerrainMaterialRenderData;

    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_InstancesRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>              m_InstanceMaterialRenderData;

    // DEBUG
    std::shared_ptr<Engine::CrossPlatformShaderProgram>                        m_ScreenTextureShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass>                           m_ScreenTextureRenderPass;
    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_ScreenRenderData;
    // DEBUG

    std::array<float, 8> m_CascadeDistances = {0.0f, 16.0f, 6.0f, 32.0f, 22.0f, 64.0f, 54.0f, 100.0f};
};