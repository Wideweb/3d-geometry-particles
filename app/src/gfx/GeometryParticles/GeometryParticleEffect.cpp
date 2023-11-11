#include "GeometryParticleEffect.hpp"

#include "ModelFactory.hpp"

void GeometryParticleEffect::bind() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    Engine::Mesh monkey = Engine::ModelLoader::loadObj("./../assets/models/monkey.obj");
    Engine::Mesh bug    = Engine::ModelLoader::loadObj("./../assets/models/bug.obj");

    render.registerGeometry("geometry-particles", {"monkey", "bug"}, {monkey, bug});

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"cbCommon",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbObject",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbMaterial", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA}
    };
    m_Shader = render.createShaderProgram(
        "./../assets/shaders/dx/light.hlsl", "./../assets/shaders/dx/light-no-shadow.hlsl", slots
    );

    Engine::CrossPlatformRenderPass::PipelineDesc pipelineDesc;
    pipelineDesc.cullMode        = Engine::CULL_MODE::BACK;
    pipelineDesc.depthClipEnable = true;
    pipelineDesc.depthFunc       = Engine::DEPTH_FUNC::LESS;

    m_RenderPass = render.createRenderPass(m_Shader, pipelineDesc);

    m_GeometryRenderData         = render.createShaderProgramDataBuffer(sizeof(RenderItemData));
    m_GeometryMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));
    m_ParticleMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));

    m_Geometry = std::make_shared<Geometry>(monkey);
    m_Particles.reserve(200);
    for (size_t i = 0; i < 200; i++) {
        m_Particles.emplace_back(*m_Geometry);
        m_Particles[i].setUp();
        m_ParticlesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
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
    glm::mat4 geometryTransform = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
    geometryTransform           = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 5.0f, 5.0f)) * geometryTransform;

    RenderItemData geometryItemData;
    geometryItemData.model = glm::transpose(geometryTransform);
    m_GeometryRenderData->copyData(&geometryItemData);

    GfxEffect::RenderMaterialData geometryMaterial;
    geometryMaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    geometryMaterial.fresnelR0     = glm::vec3(0.01f);
    geometryMaterial.roughness     = 0.5f;
    m_GeometryMaterialRenderData->copyData(&geometryMaterial);

    glm::mat4 particleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));

    for (size_t i = 0; i < m_Particles.size(); i++) {
        RenderItemData itemData;
        itemData.model = glm::transpose(geometryTransform * m_Particles[i].getTransform() * particleTransform);
        m_ParticlesRenderData[i]->copyData(&itemData);
    }

    GfxEffect::RenderMaterialData particleMaterial;
    particleMaterial.diffuseAlbedo = glm::vec4(0.25f, 0.75f, 0.1f, 1.0f);
    particleMaterial.fresnelR0     = glm::vec3(0.1f);
    particleMaterial.roughness     = 0.0f;
    m_ParticleMaterialRenderData->copyData(&particleMaterial);
    ///////////////////////////// UPDATE GPU DATA //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// DRAW ///////////////////////////////////
    render.setRenderPass(m_RenderPass);

    m_Shader->setDataSlot(0, commonData);

    m_Shader->setDataSlot(2, m_GeometryMaterialRenderData);
    m_Shader->setDataSlot(1, m_GeometryRenderData);
    render.drawItem("geometry-particles", "monkey");

    m_Shader->setDataSlot(2, m_ParticleMaterialRenderData);
    for (size_t i = 0; i < m_Particles.size(); i++) {
        m_Shader->setDataSlot(1, m_ParticlesRenderData[i]);
        render.drawItem("geometry-particles", "bug");
    }
}