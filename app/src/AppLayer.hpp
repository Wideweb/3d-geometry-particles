#pragma once

#include "Engine.hpp"

#include "GfxEffect.hpp"
#include "SkyboxEffect.hpp"
#include "GeometryParticleEffect.hpp"
#include "CascadeShadowEffect.hpp"

#include <glm/glm.hpp>
#include <vector>

class AppLayer : public Engine::Layer {
  public:
    using Layer::Layer;

    virtual void onAttach() override;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onDetach() override;
    virtual void onMouseEvent(Engine::MouseEvent &event) override;

  private:
    glm::vec2 m_MousePos;
    float m_Time = 0.0f;

    GfxEffect::RenderCommonData m_CommonData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_CommonDataBuffer;

    std::vector<std::shared_ptr<GfxEffect>> m_Effects;
};
