#pragma once

#include "Engine.hpp"
#include "Geometry.hpp"
#include "GeometryParticle.hpp"

#include <glm/mat4x4.hpp>
#include <vector>

class AppLayer : public Engine::Layer {
  private:
    Engine::Shader m_Shader, m_SurfaceShader;

    Engine::Texture m_SandTexture, m_SkyTexture;

    std::shared_ptr<Engine::Model> m_SurfaceModel;
    glm::mat4 m_SurfaceTransform = glm::mat4(1.0f);

    std::shared_ptr<Engine::Model> m_GeometryModel;
    glm::mat4 m_GeometryTransform = glm::mat4(1.0f);

    std::shared_ptr<Engine::Model> m_ParticleModel;
    glm::mat4 m_ParticleTransform = glm::mat4(1.0f);

    std::shared_ptr<Geometry> m_Geometry;
    std::vector<GeometryParticle> m_Particles;

    float m_Time = 0.0f;

  public:
    using Layer::Layer;

    virtual void onAttach() override;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onDetach() override;
    virtual void onMouseEvent(Engine::MouseEvent &event) override;
};
