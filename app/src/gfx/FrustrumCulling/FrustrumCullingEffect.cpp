#include "FrustrumCullingEffect.hpp"

#include "CascadeShadow.hpp"
#include "Frustrum.hpp"
#include "ModelFactory.hpp"

void FrustrumCullingEffect::bind() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    Engine::Mesh monkey = Engine::ModelLoader::loadObj("./../assets/models/monkey.obj");

    render.registerGeometry("frustrum-culling", {"instance"}, {monkey});

    m_FrustrumVertices.resize(8);
    m_Grid      = std::make_unique<Grid<size_t>>(AABB(glm::vec3(-1000.0f), glm::vec3(1000.0f)));
    m_ModelAABB = AABB(monkey);

    initRenderPass();
    initInstances();
}

void FrustrumCullingEffect::initRenderPass() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"cbCommon",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbObject",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbMaterial", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA}
    };
    m_Shader = render.createShaderProgram(
        "./../assets/shaders/dx/light-no-shadow.hlsl", "./../assets/shaders/dx/light-no-shadow.hlsl", slots
    );

    Engine::CrossPlatformRenderPass::PipelineDesc pipelineDesc;
    pipelineDesc.cullMode        = Engine::CULL_MODE::BACK;
    pipelineDesc.depthClipEnable = true;
    pipelineDesc.depthFunc       = Engine::DEPTH_FUNC::LESS;

    m_RenderPass = render.createRenderPass(m_Shader, pipelineDesc);
}

void FrustrumCullingEffect::initInstances() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    for (size_t i = 0; i < 10000; i++) {
        m_InstancesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }
    m_InstanceMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));

    for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
        int       x   = i / 100;
        int       z   = i % 100 + 1;
        glm::vec3 pos = glm::vec3(x * 10.0f, 0.0f, z * 10.0f);

        float uv_x = x * 5.0f / m_InstancesRenderData.size();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

        RenderItemData itemData;
        itemData.model = glm::transpose(model);
        m_InstancesRenderData[i]->copyData(&itemData);

        m_Grid->addShape(i, m_ModelAABB.move(pos));
    }

    GfxEffect::RenderMaterialData instanceMaterial;
    instanceMaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    instanceMaterial.fresnelR0     = glm::vec3(0.01f);
    instanceMaterial.roughness     = 0.5f;
    m_InstanceMaterialRenderData->copyData(&instanceMaterial);
}

void FrustrumCullingEffect::update(GfxEffect::RenderCommonData& commonData) {}

void FrustrumCullingEffect::draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) {
    auto& app    = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& render = app.getRender();
    auto& time   = app.getTime();

    Frustrum::getInWorldSpace(camera, m_FrustrumVertices);
    std::vector<CollisionShape<size_t>> visible = m_Grid->findNeighbors(m_FrustrumVertices);

    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// DRAW ///////////////////////////////////
    render.setRenderPass(m_RenderPass);

    m_Shader->setDataSlot(0, commonData);

    m_Shader->setDataSlot(2, m_InstanceMaterialRenderData);
    for (auto item : visible) {
        m_Shader->setDataSlot(1, m_InstancesRenderData[item.id]);
        render.drawItem("frustrum-culling", "instance");
    }
}