#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "CascadeShadowEffect.hpp"
#include "Engine.hpp"
#include "GeometryParticleEffect.hpp"
#include "GfxEffect.hpp"
#include "SkyboxEffect.hpp"
#include "SkyboxNoiseEffect.hpp"
#include "FrustrumCullingEffect.hpp"
#include "FrustrumCullingGPUEffect.hpp"

class AppLayer : public Engine::Layer {
public:
    using Layer::Layer;

    virtual void onAttach() override;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onDetach() override;
    virtual void onMouseEvent(Engine::MouseEvent& event) override;

private:
    glm::vec2 m_MousePos;

    GfxEffect::RenderCommonData                                   m_CommonData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_CommonDataBuffer;

    std::vector<std::shared_ptr<GfxEffect>> m_Effects;
};
