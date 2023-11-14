#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

#include "Geometry.hpp"
#include "GeometryParticle.hpp"
#include "GfxEffect.hpp"

class GeometryParticleEffect : public GfxEffect {
public:
    void bind() override;
    void update(GfxEffect::RenderCommonData& commonData) override;
    void draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) override;

private:
    struct RenderItemData {
        glm::mat4 model;
    };

    void initLightPass();
    void initDebugPass();
    
    void initGeometry();
    void initParticles();

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_Shader;
    std::shared_ptr<Engine::CrossPlatformRenderPass>    m_RenderPass;

    std::shared_ptr<Geometry>                                     m_Geometry;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_GeometryRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_GeometryMaterialRenderData;
    glm::mat4                                                     m_GeometryTransform;

    std::vector<GeometryParticle>                                              m_Particles;
    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_ParticlesRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>              m_ParticleMaterialRenderData;

    // DEBUG
    std::shared_ptr<Engine::CrossPlatformShaderProgram>                        m_DebugShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass>                           m_DebugRenderPass;
    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_DebugPlanesRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>              m_DebugPlaneMaterialRenderData;
    // DEBUG
};