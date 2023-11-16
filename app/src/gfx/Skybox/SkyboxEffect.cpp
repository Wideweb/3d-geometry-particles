#include "SkyboxEffect.hpp"

#include "ModelFactory.hpp"

void SkyboxEffect::bind() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    Engine::Mesh box = ModelFactory::createCube(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    render.registerGeometry("skybox", {"box"}, {box});

    m_SkyboxCubeTexture = render.loadCubeTexture(
        {"./../assets/skybox/right.jpg", "./../assets/skybox/left.jpg", "./../assets/skybox/top.jpg",
         "./../assets/skybox/bottom.jpg", "./../assets/skybox/front.jpg", "./../assets/skybox/back.jpg"}
    );

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"cbCommon", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA   },
        {"cbObject", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA   },
        {"cubeMap",  Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE},
    };
    m_SkyboxShader =
        render.createShaderProgram("./../assets/shaders/dx/skybox.hlsl", "./../assets/shaders/dx/skybox.hlsl", slots);

    Engine::CrossPlatformRenderPass::PipelineDesc pipelineDesc;
    pipelineDesc.cullMode        = Engine::CULL_MODE::NONE;
    pipelineDesc.depthClipEnable = true;
    pipelineDesc.depthFunc       = Engine::DEPTH_FUNC::LESS_EQUAL;

    m_SkyboxRenderPass = render.createRenderPass(m_SkyboxShader, pipelineDesc);

    m_SkyboxRenderData = render.createShaderProgramDataBuffer(sizeof(RenderItemData));
}

void SkyboxEffect::update(GfxEffect::RenderCommonData& commonData) {}

void SkyboxEffect::draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) {
    auto& app    = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& render = app.getRender();
    auto& time   = app.getTime();

    RenderItemData skyboxData;
    skyboxData.model = glm::mat4(1.0f);
    m_SkyboxRenderData->copyData(&skyboxData);

    render.setPass(m_SkyboxRenderPass);
    m_SkyboxShader->setDataSlot(0, commonData);
    m_SkyboxShader->setDataSlot(1, m_SkyboxRenderData);
    m_SkyboxShader->setTextureSlot(2, m_SkyboxCubeTexture);
    render.drawItem("skybox", "box");
}