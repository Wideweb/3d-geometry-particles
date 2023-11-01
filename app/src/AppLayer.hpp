#pragma once

#include "Engine.hpp"

#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>

struct RenderItemData {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
};

class AppLayer : public Engine::Layer {
  private:
    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_Shader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_RenderPass;

    glm::mat4 m_WorldTransform = glm::mat4(1.0f);

    

    // Engine::Shader m_Shader, m_SurfaceShader;

    // Engine::Texture m_SandTexture, m_SkyTexture;

    // std::shared_ptr<Engine::Model> m_SurfaceModel;
    // glm::mat4 m_SurfaceTransform = glm::mat4(1.0f);

    // std::shared_ptr<Engine::Model> m_GeometryModel;
    // glm::mat4 m_GeometryTransform = glm::mat4(1.0f);

    // std::shared_ptr<Engine::Model> m_ParticleModel;
    // glm::mat4 m_ParticleTransform = glm::mat4(1.0f);

    // std::shared_ptr<Geometry> m_Geometry;
    // std::vector<GeometryParticle> m_Particles;

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
