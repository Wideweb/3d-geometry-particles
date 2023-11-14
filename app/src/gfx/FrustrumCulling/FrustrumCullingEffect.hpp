#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

#include "GfxEffect.hpp"
#include "Grid.hpp"

class FrustrumCullingEffect : public GfxEffect {
public:
    void bind() override;
    void update(GfxEffect::RenderCommonData& commonData) override;
    void draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) override;

private:
    struct RenderItemData {
        glm::mat4 model;
    };

    void initRenderPass();
    void initInstances();

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_Shader;
    std::shared_ptr<Engine::CrossPlatformRenderPass>    m_RenderPass;

    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_InstancesRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>              m_InstanceMaterialRenderData;

    std::vector<glm::vec3>      m_FrustrumVertices;
    std::unique_ptr<Grid<size_t>> m_Grid;
    AABB                          m_ModelAABB;
};