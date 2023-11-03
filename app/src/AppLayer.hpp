#pragma once

#include "Engine.hpp"

#include "Geometry.hpp"
#include "GeometryParticle.hpp"

#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>

struct RenderCommonData {
    glm::mat4 view;
    glm::mat4 projection;
};

struct RenderItemDataTexturePass {
  glm::mat4 model;
};

struct RenderItemDataColorPass {
  glm::mat4 model;
  glm::vec4 color;
};

class AppLayer : public Engine::Layer {
  private:
    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_ShaderTexture;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_RenderPassTexture;

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_ShaderColor;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_RenderPassColor;

    std::shared_ptr<Engine::CrossPlatformTexture> m_Texture;

    glm::mat4 m_WorldTransform = glm::mat4(1.0f);

    glm::vec2 m_MousePos;

    // Engine::Shader m_Shader, m_SurfaceShader;

    // Engine::Texture m_SandTexture, m_SkyTexture;

    // std::shared_ptr<Engine::Model> m_SurfaceModel;
    // glm::mat4 m_SurfaceTransform = glm::mat4(1.0f);

    // std::shared_ptr<Engine::Model> m_GeometryModel;
    glm::mat4 m_GeometryTransform = glm::mat4(1.0f);

    // std::shared_ptr<Engine::Model> m_ParticleModel;
    glm::mat4 m_ParticleTransform = glm::mat4(1.0f);

    std::shared_ptr<Geometry> m_Geometry;
    std::vector<GeometryParticle> m_Particles;

    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_CommonRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_GeometryRenderData;
    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_ParticlesRenderData;

    float m_Time = 0.0f;

    Engine::Mesh createCube(float left, float right, float bottom, float top, float back, float front);
    Engine::Mesh createCube2();

  public:
    using Layer::Layer;

    virtual void onAttach() override;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onDetach() override;
    virtual void onMouseEvent(Engine::MouseEvent &event) override;
};
