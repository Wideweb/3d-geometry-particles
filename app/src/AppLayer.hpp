#pragma once

#include "Engine.hpp"
#include "GeometryParticle.hpp"

#include <glm/mat4x4.hpp>
#include <vector>

class AppLayer : public Engine::Layer {
  private:
    Engine::Shader m_Shader;

    std::shared_ptr<Engine::Model> m_GeometryModel;
    glm::mat4 m_GeometryTransform = glm::mat4(1.0f);

    std::shared_ptr<Engine::Model> m_ParticleModel;
    glm::mat4 m_ParticleTransform = glm::mat4(1.0f);

    std::vector<GeometryParticle> m_Particles;

  public:
    using Layer::Layer;

    virtual void onAttach() override;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onDetach() override;
    virtual void onMouseEvent(Engine::MouseEvent &event) override;
};
