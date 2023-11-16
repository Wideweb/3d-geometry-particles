#include "GeometryParticleEffect.hpp"

#include "ModelFactory.hpp"

void GeometryParticleEffect::bind() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    Engine::Mesh monkey = Engine::ModelLoader::loadObj("./../assets/models/monkey.obj");
    Engine::Mesh bug    = Engine::ModelLoader::loadObj("./../assets/models/bug.obj");
    Engine::Mesh plane  = ModelFactory::createPlane(4.0f, 1, 1);

    m_Geometry = std::make_shared<Geometry>(monkey);

    render.registerGeometry("geometry-particles", {"monkey", "bug", "plane"}, {monkey, bug, plane});

    initLightPass();

    initGeometry();
    initParticles();

    initDebugPass();
}

void GeometryParticleEffect::initLightPass() {
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

void GeometryParticleEffect::initDebugPass() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    std::vector<Engine::ShaderProgramSlotDesc> debugSlots = {
        {"cbCommon",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbObject",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbMaterial", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA}
    };
    m_DebugShader = render.createShaderProgram(
        "./../assets/shaders/dx/rgba.hlsl", "./../assets/shaders/dx/rgba.hlsl", debugSlots
    );

    Engine::CrossPlatformRenderPass::PipelineDesc debugPipelineDesc;
    debugPipelineDesc.cullMode        = Engine::CULL_MODE::NONE;
    debugPipelineDesc.depthClipEnable = true;
    debugPipelineDesc.depthFunc       = Engine::DEPTH_FUNC::LESS;

    m_DebugRenderPass = render.createRenderPass(m_DebugShader, debugPipelineDesc);

    GfxEffect::RenderMaterialData debugPlaneMaterial;
    debugPlaneMaterial.diffuseAlbedo = glm::vec4(1.0f, 0.25, 0.25, 1.0f);
    m_DebugPlaneMaterialRenderData   = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));
    m_DebugPlaneMaterialRenderData->copyData(&debugPlaneMaterial);

    for (size_t i = 0; i < m_Particles.size(); i++) {
        RenderItemData itemData;
        itemData.model = m_GeometryTransform * m_Particles[i].getTransform() * glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(0.0f, 1.0f, 0.0f));
        itemData.model = glm::transpose(itemData.model);
        m_DebugPlanesRenderData[i]->copyData(&itemData);
    }
}

void GeometryParticleEffect::initGeometry() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    m_GeometryTransform = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
    m_GeometryTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 5.0f, 5.0f)) * m_GeometryTransform;

    RenderItemData geometryItemData;
    geometryItemData.model = glm::transpose(m_GeometryTransform);
    m_GeometryRenderData   = render.createShaderProgramDataBuffer(sizeof(RenderItemData));
    m_GeometryRenderData->copyData(&geometryItemData);

    GfxEffect::RenderMaterialData geometryMaterial;
    geometryMaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    geometryMaterial.fresnelR0     = glm::vec3(0.01f);
    geometryMaterial.roughness     = 0.5f;
    m_GeometryMaterialRenderData   = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));
    m_GeometryMaterialRenderData->copyData(&geometryMaterial);
}

void GeometryParticleEffect::initParticles() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    GfxEffect::RenderMaterialData particleMaterial;
    particleMaterial.diffuseAlbedo = glm::vec4(0.25f, 0.75f, 0.1f, 1.0f);
    particleMaterial.fresnelR0     = glm::vec3(0.1f);
    particleMaterial.roughness     = 0.0f;
    m_ParticleMaterialRenderData   = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));
    m_ParticleMaterialRenderData->copyData(&particleMaterial);

    m_Particles.reserve(1);
    for (size_t i = 0; i < 1; i++) {
        m_Particles.emplace_back(*m_Geometry);
        m_Particles[i].setUp();
        m_ParticlesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
        m_DebugPlanesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }
}

void GeometryParticleEffect::update(GfxEffect::RenderCommonData& commonData) {
    for (auto& particle : m_Particles) {
        particle.update();
    }
}

void GeometryParticleEffect::draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) {
    auto& app    = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& render = app.getRender();

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// UPDATE GPU DATA //////////////////////////////
    glm::mat4 particleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));

    for (size_t i = 0; i < m_Particles.size(); i++) {
        RenderItemData itemData;
        itemData.model = glm::transpose(m_GeometryTransform * m_Particles[i].getTransform() * particleTransform);
        m_ParticlesRenderData[i]->copyData(&itemData);
    }
    ///////////////////////////// UPDATE GPU DATA //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// DRAW ///////////////////////////////////
    render.setPass(m_RenderPass);

    m_Shader->setDataSlot(0, commonData);

    m_Shader->setDataSlot(2, m_GeometryMaterialRenderData);
    m_Shader->setDataSlot(1, m_GeometryRenderData);
    render.drawItem("geometry-particles", "monkey");

    m_Shader->setDataSlot(2, m_ParticleMaterialRenderData);
    for (size_t i = 0; i < m_Particles.size(); i++) {
        m_Shader->setDataSlot(1, m_ParticlesRenderData[i]);
        render.drawItem("geometry-particles", "bug");
    }
    /////////////////////////////////// DRAW ///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// DEBUG ///////////////////////////////////
    render.setPass(m_DebugRenderPass);

    m_Shader->setDataSlot(0, commonData);
    m_Shader->setDataSlot(2, m_DebugPlaneMaterialRenderData);
    for (size_t i = 0; i < m_DebugPlanesRenderData.size(); i++) {
        m_Shader->setDataSlot(1, m_DebugPlanesRenderData[i]);
        render.drawItem("geometry-particles", "plane");
    }
}