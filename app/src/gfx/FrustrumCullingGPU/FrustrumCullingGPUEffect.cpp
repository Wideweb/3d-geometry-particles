#include "FrustrumCullingGPUEffect.hpp"

#include "CascadeShadow.hpp"
#include "Frustrum.hpp"
#include "ModelFactory.hpp"

namespace FrustrumCullingGPU {

void Effect::bind() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    Engine::Mesh monkey = Engine::ModelLoader::loadObj("./../assets/models/monkey.obj");

    render.registerGeometry("frustrum-culling", {"instance"}, {monkey});

    m_FrustrumVertices.resize(8);
    m_ModelAABB = AABB(monkey);

    initRenderPass();
    initComputePass();
    initInstances();
}

void Effect::initRenderPass() {
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

void Effect::initComputePass() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"frustrum",  Engine::SHADER_PROGRAM_SLOT_TYPE::DATA           },
        {"instances", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA_ARRAY     },
        {"visible",   Engine::SHADER_PROGRAM_SLOT_TYPE::READ_WRITE_DATA},
    };
    m_ComputeShader = render.createComputeProgram("./../assets/shaders/dx/frustrum-culling.hlsl", slots);
    m_ComputePass = render.createComputePass(m_ComputeShader);
}

void Effect::initInstances() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    for (size_t i = 0; i < 256 * 25; i++) {
        m_InstancesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }
    m_InstanceMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));

    std::vector<AABB> aabbs;
    aabbs.reserve(m_InstancesRenderData.size());

    for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
        int       x   = i / 100;
        int       z   = i % 100 + 1;
        glm::vec3 pos = glm::vec3(x * 10.0f, 0.0f, z * 10.0f);

        float uv_x = x * 5.0f / m_InstancesRenderData.size();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

        RenderItemData itemData;
        itemData.model = glm::transpose(model);
        m_InstancesRenderData[i]->copyData(&itemData);

        aabbs.push_back(m_ModelAABB.move(pos));
    }

    GfxEffect::RenderMaterialData instanceMaterial;
    instanceMaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    instanceMaterial.fresnelR0     = glm::vec3(0.01f);
    instanceMaterial.roughness     = 0.5f;
    m_InstanceMaterialRenderData->copyData(&instanceMaterial);

    m_FrustrumComputeData  = render.createShaderProgramDataBuffer(sizeof(AABB));
    m_InstancesComputeData = render.createShaderProgramDataBuffer(sizeof(AABB) * m_InstancesRenderData.size());
    m_VisibleComputeData   = render.createReadWriteDataBuffer(sizeof(uint32_t) * (m_InstancesRenderData.size() + 1));

    m_InstancesComputeData->copyData(aabbs.data());

    std::vector<uint32_t> visibleIndices(m_InstancesRenderData.size() + 1, 0);
    m_VisibleComputeData->copyData(visibleIndices.data());
}

void Effect::update(GfxEffect::RenderCommonData& commonData) {}

void Effect::draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) {
    auto& app    = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& render = app.getRender();
    auto& time   = app.getTime();

    Frustrum::getInWorldSpace(camera, m_FrustrumVertices, 0.5f);
    AABB frustrumAABB(m_FrustrumVertices);
    m_FrustrumComputeData->copyData(&frustrumAABB);

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// COMPUTE //////////////////////////////////
    render.setComputePass(m_ComputePass);

    uint32_t zero = 0;
    m_VisibleComputeData->copyData(&zero, sizeof(uint32_t));
    m_VisibleComputeData->writeToVRAM();

    m_ComputeShader->setDataSlot(0, m_FrustrumComputeData);
    m_ComputeShader->setDataArraySlot(1, m_InstancesComputeData);
    m_ComputeShader->setReadWriteDataSlot(2, m_VisibleComputeData);

    size_t numGroupsX = static_cast<size_t>(std::ceilf(m_InstancesRenderData.size() / 256.0f));
    render.compute(numGroupsX, 1, 1);

    m_VisibleComputeData->readFromVRAM();
    uint32_t* visible = static_cast<uint32_t*>(m_VisibleComputeData->data());
    uint32_t visibleNum = visible[0];
    ///////////////////////////////// COMPUTE //////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// DRAW ///////////////////////////////////
    render.setPass(m_RenderPass);

    m_Shader->setDataSlot(0, commonData);

    m_Shader->setDataSlot(2, m_InstanceMaterialRenderData);
    for (uint32_t index = 1; index <= visibleNum; index++) {
        uint32_t id = visible[index];
        m_Shader->setDataSlot(1, m_InstancesRenderData[id]);
        render.drawItem("frustrum-culling", "instance");
    }
}

}  // namespace FrustrumCullingGPU