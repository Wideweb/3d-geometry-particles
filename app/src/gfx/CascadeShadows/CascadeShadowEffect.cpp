#include "CascadeShadowEffect.hpp"

#include "CascadeShadow.hpp"
#include "ModelFactory.hpp"

void CascadeShadowEffect::bind() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    Engine::Mesh monkey  = Engine::ModelLoader::loadObj("./../assets/models/monkey.obj");
    Engine::Mesh terrain = ModelFactory::createPlane(20.0f, 10, 10);
    Engine::Mesh plane   = ModelFactory::createPlane(2.0f, 1, 1);

    render.registerGeometry("cascade-shadow", {"instance", "terrain", "plane"}, {monkey, terrain, plane});

    initDepthPass();
    initLightPass();

    initTerrain();
    initInstances();

    // initDebugPass();
}

void CascadeShadowEffect::initDepthPass() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"cbCommon", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbObject", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
    };
    m_DepthShader = render.createShaderProgram("./../assets/shaders/dx/depth.hlsl", "", slots);

    for (size_t i = 0; i < m_DepthMaps.size(); i++) {
        m_DepthMaps[i].texture = render.createDepthStencilTexture(2048, 2048);

        m_DepthMaps[i].framebuffer = render.createFramebuffer();
        m_DepthMaps[i].framebuffer->setDSAttachment(m_DepthMaps[i].texture);

        m_DepthMaps[i].renderData = render.createShaderProgramDataBuffer(sizeof(DepthRenderCommonData));
    }

    Engine::CrossPlatformRenderPass::PipelineDesc pipelineDesc;
    pipelineDesc.cullMode        = Engine::CULL_MODE::FRONT;
    pipelineDesc.depthClipEnable = true;
    pipelineDesc.depthFunc       = Engine::DEPTH_FUNC::LESS;

    std::vector<Engine::CROSS_PLATFROM_TEXTURE_FORMATS> rtvs = {};

    m_DepthRenderPass =
        render.createRenderPass(m_DepthShader, rtvs, Engine::CROSS_PLATFROM_TEXTURE_FORMATS::D24S8, pipelineDesc);
}

void CascadeShadowEffect::initLightPass() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"cbCommon",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA           },
        {"cbObject",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA           },
        {"cbMaterial", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA           },
        {"shadowMap",  Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE_ARRAY_4},
    };
    m_LightShader = render.createShaderProgram(
        "./../assets/shaders/dx/light-cascade-shadow.hlsl", "./../assets/shaders/dx/light-cascade-shadow.hlsl", slots
    );

    Engine::CrossPlatformRenderPass::PipelineDesc rsLightPipelineDesc;
    rsLightPipelineDesc.cullMode        = Engine::CULL_MODE::BACK;
    rsLightPipelineDesc.depthClipEnable = true;
    rsLightPipelineDesc.depthFunc       = Engine::DEPTH_FUNC::LESS;

    m_LightRenderPass = render.createRenderPass(m_LightShader, rsLightPipelineDesc);
}

void CascadeShadowEffect::initDebugPass() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"cbObject",   Engine::SHADER_PROGRAM_SLOT_TYPE::DATA   },
        {"diffuseMap", Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE},
    };
    m_ScreenTextureShader = render.createShaderProgram(
        "./../assets/shaders/dx/screen-texture.hlsl", "./../assets/shaders/dx/screen-texture.hlsl", slots
    );

    Engine::CrossPlatformRenderPass::PipelineDesc pipelineDesc;
    pipelineDesc.cullMode        = Engine::CULL_MODE::NONE;
    pipelineDesc.depthClipEnable = false;
    pipelineDesc.depthFunc       = Engine::DEPTH_FUNC::LESS;

    m_ScreenTextureRenderPass = render.createRenderPass(m_ScreenTextureShader, pipelineDesc);

    for (size_t i = 0; i < 4; i++) {
        m_ScreenRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }
}

void CascadeShadowEffect::initTerrain() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    m_TerrainRenderData         = render.createShaderProgramDataBuffer(sizeof(RenderCommonData));
    m_TerrainMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));
    
    RenderItemData terrainItemData;
    terrainItemData.model = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
    terrainItemData.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) * terrainItemData.model;
    terrainItemData.model = glm::transpose(terrainItemData.model);
    m_TerrainRenderData->copyData(&terrainItemData);

    GfxEffect::RenderMaterialData terrainMaterial;
    terrainMaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    terrainMaterial.fresnelR0     = glm::vec3(0.01f);
    terrainMaterial.roughness     = 0.5f;
    m_TerrainMaterialRenderData->copyData(&terrainMaterial);
}

void CascadeShadowEffect::initInstances() {
    auto& app    = Engine::Application::get();
    auto& render = app.getRender();

    for (size_t i = 0; i < 100; i++) {
        m_InstancesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }
    m_InstanceMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderMaterialData));

    for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
        int x = i / 5;
        int z = i % 5 + 1;

        float uv_x = x * 5.0f / m_InstancesRenderData.size();

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        model           = glm::translate(glm::mat4(1.0f), glm::vec3(x * 5.0f, 0.0f, z * 5.0f)) * model;

        RenderItemData itemData;
        itemData.model = glm::transpose(model);
        m_InstancesRenderData[i]->copyData(&itemData);
    }

    GfxEffect::RenderMaterialData instanceMaterial;
    instanceMaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    instanceMaterial.fresnelR0     = glm::vec3(0.01f);
    instanceMaterial.roughness     = 0.5f;
    m_InstanceMaterialRenderData->copyData(&instanceMaterial);
}

void CascadeShadowEffect::update(GfxEffect::RenderCommonData& commonData) {
    auto& app    = Engine::Application::get();
    auto& camera = app.getCamera();
    
    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    glm::mat4 projFix = glm::mat4(
        0.5f,  0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, 0.0f, 1.0f
    );

    auto cascades = CascadeShadow::calculate(camera, commonData.light.view, m_CascadeDistances, -10.0f, 10.0f);

    for (size_t i = 0; i < m_DepthMaps.size(); i++) {
        DepthRenderCommonData depthCommonData;
        depthCommonData.viewProj = glm::transpose(cascades[i].viewProj);
        m_DepthMaps[i].renderData->copyData(&depthCommonData);
    }

    for (size_t i = 0; i < cascades.size(); i++) {
        commonData.light.cascades[i] = glm::transpose(projFix * cascades[i].viewProj);
        commonData.light.cascadesFrontPlanes[i] = cascades[i].frontPlane;
    }
}

void CascadeShadowEffect::draw(std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> commonData) {
    auto& app    = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& render = app.getRender();
    auto& time   = app.getTime();

    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// DEPTH MAP /////////////////////////////////
    uint32_t prevViewportWidth, prevViewportHeight;
    render.getViewport(prevViewportWidth, prevViewportHeight);

    render.setPass(m_DepthRenderPass);
    render.setViewport(2048, 2048);

    for (size_t i = 0; i < m_DepthMaps.size(); i++) {
        render.setFramebuffer(m_DepthMaps[i].framebuffer);
        render.clear(0.0f, 0.0f, 0.0f, 0.0f);

        m_DepthShader->setDataSlot(0, m_DepthMaps[i].renderData);

        m_DepthShader->setDataSlot(1, m_TerrainRenderData);
        render.drawItem("cascade-shadow", "terrain");

        for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
            m_DepthShader->setDataSlot(1, m_InstancesRenderData[i]);
            render.drawItem("cascade-shadow", "instance");
        }
    }
    render.setFramebuffer(nullptr);
    render.setViewport(prevViewportWidth, prevViewportHeight);
    //////////////////////////////// DEPTH MAP /////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// DRAW ///////////////////////////////////
    render.setPass(m_LightRenderPass);

    m_LightShader->setDataSlot(0, commonData);

    // Array
    m_LightShader->setTextureSlot(3, m_DepthMaps[0].texture);

    m_LightShader->setDataSlot(2, m_TerrainMaterialRenderData);
    m_LightShader->setDataSlot(1, m_TerrainRenderData);
    render.drawItem("cascade-shadow", "terrain");

    m_LightShader->setDataSlot(2, m_InstanceMaterialRenderData);
    for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
        m_LightShader->setDataSlot(1, m_InstancesRenderData[i]);
        render.drawItem("cascade-shadow", "instance");
    }
    /////////////////////////////////// DRAW ///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// DEBUG ///////////////////////////////////
    // render.setPass(m_ScreenTextureRenderPass);
    // for (size_t i = 0; i < m_ScreenRenderData.size(); i++) {
    //     RenderItemData itemData;
    //     itemData.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 1.0f));
    //     itemData.model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7 + (i) * 0.45f, -0.7f, 0.0f)) * itemData.model;
    //     m_ScreenRenderData[i]->copyData(&itemData);

    //     m_ScreenTextureShader->setDataSlot(0, m_ScreenRenderData[i]);
    //     m_ScreenTextureShader->setTextureSlot(1, m_DepthMaps[i].texture);
    //     render.drawItem("cascade-shadow", "plane");
    // }
}