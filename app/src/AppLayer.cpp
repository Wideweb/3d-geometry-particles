#include "AppLayer.hpp"

#include "TBN.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include <algorithm>
#include <cmath>
#include <unordered_map>

void AppLayer::onAttach() {
    auto &app = Engine::Application::get();
    auto &camera = app.getCamera();
    auto &render = app.getRender();

    render.beginInitialization();

    Engine::Mesh box = createCube(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    Engine::Mesh bug = Engine::ModelLoader::loadObj("./../assets/models/bug.obj");
    Engine::Mesh monkey = Engine::ModelLoader::loadObj("./../assets/models/monkey.obj");
    Engine::Mesh plane = createPlane(2.0f, 1, 1);
    Engine::Mesh terrain = createPlane(50.0f, 10, 10);

    render.registerGeometry("world", {"monkey", "bug", "box", "plane", "terrain"}, {monkey, bug, box, plane, terrain});

    ////////////////////////////////////////////////
    /////////////////// SKY BOX ////////////////////
    m_SkyboxCubeTexture = render.loadCubeTexture({"./../assets/skybox/right.jpg", "./../assets/skybox/left.jpg",
                                                  "./../assets/skybox/top.jpg", "./../assets/skybox/bottom.jpg",
                                                  "./../assets/skybox/front.jpg", "./../assets/skybox/back.jpg"});

    std::vector<Engine::ShaderProgramSlotDesc> slotsSkyboxPass = {
        {"cbCommon", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbObject", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cubeMap", Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE},
    };
    m_SkyboxShader = render.createShaderProgram("./../assets/shaders/dx/skybox.hlsl",
                                                "./../assets/shaders/dx/skybox.hlsl", slotsSkyboxPass);

    Engine::CrossPlatformRenderPass::PipelineDesc rsSkyboxPipelineDesc;
    rsSkyboxPipelineDesc.cullMode = Engine::CULL_MODE::NONE;
    rsSkyboxPipelineDesc.depthClipEnable = true;
    rsSkyboxPipelineDesc.depthFunc = Engine::DEPTH_FUNC::LESS_EQUAL;

    m_SkyboxRenderPass = render.createRenderPass(m_SkyboxShader, rsSkyboxPipelineDesc);

    m_SkyboxRenderData = render.createShaderProgramDataBuffer(sizeof(RenderItemData));
    /////////////////// SKY BOX ////////////////////
    ////////////////////////////////////////////////

    m_Texture = render.loadTexture("./../assets/box-2.png");

    std::vector<Engine::ShaderProgramSlotDesc> slotsTexturePass = {
        {"cbCommon", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbObject", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"diffuseMap", Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE}};
    m_ShaderTexture = render.createShaderProgram("./../assets/shaders/dx/texture.hlsl",
                                                 "./../assets/shaders/dx/texture.hlsl", slotsTexturePass);

    Engine::CrossPlatformRenderPass::PipelineDesc rsTexturePipelineDesc;
    rsTexturePipelineDesc.cullMode = Engine::CULL_MODE::BACK;
    rsTexturePipelineDesc.depthClipEnable = true;
    rsTexturePipelineDesc.depthFunc = Engine::DEPTH_FUNC::LESS;

    m_RenderPassTexture = render.createRenderPass(m_ShaderTexture, rsTexturePipelineDesc);

    std::vector<Engine::ShaderProgramSlotDesc> slotsLightPass = {
        {"cbCommon", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbObject", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"cbMaterial", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"shadowMap", Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE_ARRAY_4},
    };
    m_LightShader = render.createShaderProgram("./../assets/shaders/dx/light.hlsl", "./../assets/shaders/dx/light.hlsl",
                                               slotsLightPass);

    Engine::CrossPlatformRenderPass::PipelineDesc rsLightPipelineDesc;
    rsLightPipelineDesc.cullMode = Engine::CULL_MODE::BACK;
    rsLightPipelineDesc.depthClipEnable = true;
    rsLightPipelineDesc.depthFunc = Engine::DEPTH_FUNC::LESS;

    m_LightRenderPass = render.createRenderPass(m_LightShader, rsLightPipelineDesc);

    m_CommonRenderData = render.createShaderProgramDataBuffer(sizeof(RenderCommonData));
    m_GeometryRenderData = render.createShaderProgramDataBuffer(sizeof(RenderCommonData));
    m_GeometryMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(Material));
    m_ParticleMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(Material));

    for (size_t i = 0; i < 100; i++) {
        m_InstancesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }

    m_TerrainRenderData = render.createShaderProgramDataBuffer(sizeof(RenderCommonData));
    m_TerrainMaterialRenderData = render.createShaderProgramDataBuffer(sizeof(Material));

    m_ParticleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));

    camera.setPosition(glm::vec3(3.0f, 1.5f, 3.0f));
    camera.setRotation(glm::quat(glm::vec3(glm::radians(-25.0f), glm::radians(45.0f), 0.0f)));

    m_Geometry = std::make_shared<Geometry>(monkey);
    m_Particles.reserve(200);
    for (size_t i = 0; i < 200; i++) {
        m_Particles.emplace_back(*m_Geometry);
        m_Particles[i].setUp();
        m_ParticlesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }

    initDepthPass();
    initScreenTexturePass();

    render.endInitialization();
}

void AppLayer::initScreenTexturePass() {
    auto &app = Engine::Application::get();
    auto &render = app.getRender();

    std::vector<Engine::ShaderProgramSlotDesc> slots = {
        {"cbObject", Engine::SHADER_PROGRAM_SLOT_TYPE::DATA},
        {"diffuseMap", Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE},
    };
    m_ScreenTextureShader = render.createShaderProgram("./../assets/shaders/dx/screen-texture.hlsl",
                                                       "./../assets/shaders/dx/screen-texture.hlsl", slots);

    Engine::CrossPlatformRenderPass::PipelineDesc pipelineDesc;
    pipelineDesc.cullMode = Engine::CULL_MODE::NONE;
    pipelineDesc.depthClipEnable = false;
    pipelineDesc.depthFunc = Engine::DEPTH_FUNC::LESS;

    m_ScreenTextureRenderPass = render.createRenderPass(m_ScreenTextureShader, pipelineDesc);

    for (size_t i = 0; i < 4; i++) {
        m_ScreenRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemData)));
    }
}

void AppLayer::initDepthPass() {
    auto &app = Engine::Application::get();
    auto &render = app.getRender();

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
    pipelineDesc.cullMode = Engine::CULL_MODE::FRONT;
    pipelineDesc.depthClipEnable = true;
    pipelineDesc.depthFunc = Engine::DEPTH_FUNC::LESS;

    std::vector<Engine::CROSS_PLATFROM_TEXTURE_FORMATS> rtvs = {};

    m_DepthRenderPass =
        render.createRenderPass(m_DepthShader, rtvs, Engine::CROSS_PLATFROM_TEXTURE_FORMATS::D24S8, pipelineDesc);
}

void AppLayer::onUpdate() {
    auto &app = Engine::Application::get();
    auto &camera = app.getCamera();
    auto &time = app.getTime();
    auto &input = app.getInput();
    auto &cameraController = app.getCameraController();

    if (input.IsKeyPressed(Engine::KeyCode::W)) {
        glm::vec3 delta = glm::vec3(0.0, 0.0f, 0.5f);
        cameraController.move(delta, 0.1);
    }

    if (input.IsKeyPressed(Engine::KeyCode::S)) {
        glm::vec3 delta = glm::vec3(0.0, 0.0f, -0.5f);
        cameraController.move(delta, 0.1);
    }

    if (input.IsKeyPressed(Engine::KeyCode::A)) {
        glm::vec3 delta = glm::vec3(-0.5f, 0.0f, 0.0);
        cameraController.move(delta, 0.1);
    }

    if (input.IsKeyPressed(Engine::KeyCode::D)) {
        glm::vec3 delta = glm::vec3(0.5f, 0.0f, 0.0);
        cameraController.move(delta, 0.1);
    }

    if (input.IsKeyPressed(Engine::KeyCode::Q)) {
        glm::vec3 delta = glm::vec3(0.0f, -0.5, 0.0f);
        cameraController.move(delta, 0.1);
    }

    if (input.IsKeyPressed(Engine::KeyCode::E)) {
        glm::vec3 delta = glm::vec3(0.0f, 0.5, 0.0f);
        cameraController.move(delta, 0.1);
    }

    for (auto &particle : m_Particles) {
        particle.update();
    }

    m_Time += 0.016f;
}

void AppLayer::onDraw() {
    auto &app = Engine::Application::get();
    auto &camera = app.getCamera();
    auto &render = app.getRender();
    auto &time = app.getTime();

    ////////////////////////////////////////////////
    ////////////////// UPDATE DATA /////////////////
    m_WorldTransform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
    // m_WorldTransform = glm::rotate(glm::mat4(1.0f), (float)std::sin(time.getTotalSeconds()), glm::vec3(1.0f, 0.0f,
    // 0.0f)) * m_WorldTransform; m_WorldTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) *
    // m_WorldTransform; m_WorldTransform = glm::transpose(m_WorldTransform);

    float shadowFrustumHalf = 4;

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    glm::mat4 projFix =
        glm::mat4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

    glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -5.0f, -5.0f));
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    CascadeShadow cascadeShadow;
    auto cascades = cascadeShadow.calculate(lightView);

    for (size_t i = 0; i < m_DepthMaps.size(); i++) {
        DepthRenderCommonData depthCommonData;
        depthCommonData.viewProj = glm::transpose(cascades[i].viewProj);
        m_DepthMaps[i].renderData->copyData(&depthCommonData);
    }

    RenderCommonData commonData;
    commonData.view = glm::transpose(camera.viewMatrix());
    commonData.projection = glm::transpose(camera.projectionMatrix());
    commonData.viewPos = camera.positionVec();
    commonData.ambientLight = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    commonData.light.strength = glm::vec3(1.0f);
    commonData.light.direction = lightDir;

    for (size_t i = 0; i < cascades.size(); i++) {
        commonData.light.cascades[i].viewProj = glm::transpose(projFix * cascades[i].viewProj);
        // commonData.light.cascades[i].frontPlane = cascades[i].frontPlane;
    }

    m_CommonRenderData->copyData(&commonData);

    for (size_t i = 0; i < m_Particles.size(); i++) {
        RenderItemData itemData;
        itemData.model = glm::transpose(m_WorldTransform * m_Particles[i].getTransform() * m_ParticleTransform);
        m_ParticlesRenderData[i]->copyData(&itemData);
    }

    Material pmaterial;
    pmaterial.diffuseAlbedo = glm::vec4(0.25f, 0.75f, 0.1f, 1.0f);
    pmaterial.fresnelR0 = glm::vec3(0.1f);
    pmaterial.roughness = 0.0f;
    m_ParticleMaterialRenderData->copyData(&pmaterial);

    RenderItemData gitemData;
    gitemData.model = glm::transpose(m_WorldTransform);
    m_GeometryRenderData->copyData(&gitemData);

    Material gmaterial;
    gmaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    gmaterial.fresnelR0 = glm::vec3(0.01f);
    gmaterial.roughness = 0.5f;
    m_GeometryMaterialRenderData->copyData(&gmaterial);

    RenderItemData titemData;
    titemData.model = glm::rotate(glm::mat4(1.0f), 1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
    titemData.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) * titemData.model;
    titemData.model = glm::transpose(titemData.model);
    m_TerrainRenderData->copyData(&titemData);

    Material tmaterial;
    tmaterial.diffuseAlbedo = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);
    tmaterial.fresnelR0 = glm::vec3(0.01f);
    tmaterial.roughness = 0.5f;
    m_TerrainMaterialRenderData->copyData(&tmaterial);

    for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
        int x = i / 5;
        int z = i % 5 + 1;

        float uv_x = x * 5.0f / m_InstancesRenderData.size();
        float y = std::sin(uv_x * 6.28f * 2.0 + time.getTotalSeconds()) + 1.0f;

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        model = glm::translate(glm::mat4(1.0f), glm::vec3(x * 5, 0.0f, z * 5)) * model;

        RenderItemData itemData;
        itemData.model = glm::transpose(model);
        m_InstancesRenderData[i]->copyData(&itemData);
    }
    
    ////////////////// UPDATE DATA /////////////////
    ////////////////////////////////////////////////

    ////////////////////////////////////////////////
    /////////////////// DEPTH MAP //////////////////
    render.setRenderPass(m_DepthRenderPass);
    for (size_t i = 0; i < m_DepthMaps.size(); i++) {
        render.setFramebuffer(m_DepthMaps[i].framebuffer);
        render.clear(0.0f, 0.0f, 0.0f, 0.0f);

        m_DepthShader->setDataSlot(0, m_DepthMaps[i].renderData);

        m_DepthShader->setDataSlot(1, m_TerrainRenderData);
        render.drawItem("world", "terrain");

        for (size_t i = 0; i < m_Particles.size(); i++) {
            m_DepthShader->setDataSlot(1, m_ParticlesRenderData[i]);
            render.drawItem("world", "bug");
        }

        m_DepthShader->setDataSlot(1, m_GeometryRenderData);
        render.drawItem("world", "monkey");

        for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
            m_DepthShader->setDataSlot(1, m_InstancesRenderData[i]);
            render.drawItem("world", "monkey");
        }
    }
    render.setFramebuffer(nullptr);
    /////////////////// DEPTH MAP //////////////////
    ////////////////////////////////////////////////

    render.setRenderPass(m_LightRenderPass);

    m_LightShader->setDataSlot(0, m_CommonRenderData);

    // array
    m_LightShader->setTextureSlot(3, m_DepthMaps[0].texture);

    m_LightShader->setDataSlot(2, m_TerrainMaterialRenderData);
    m_LightShader->setDataSlot(1, m_TerrainRenderData);
    render.drawItem("world", "terrain");

    m_LightShader->setDataSlot(2, m_ParticleMaterialRenderData);
    for (size_t i = 0; i < m_Particles.size(); i++) {
        m_LightShader->setDataSlot(1, m_ParticlesRenderData[i]);
        render.drawItem("world", "bug");
    }

    m_LightShader->setDataSlot(2, m_GeometryMaterialRenderData);
    m_LightShader->setDataSlot(1, m_GeometryRenderData);
    render.drawItem("world", "monkey");

    for (size_t i = 0; i < m_InstancesRenderData.size(); i++) {
        m_LightShader->setDataSlot(1, m_InstancesRenderData[i]);
        render.drawItem("world", "monkey");
    }

    ////////////////////////////////////////////////
    /////////////////// SKY BOX ////////////////////
    RenderItemData skyboxData;
    skyboxData.model = glm::mat4(1.0f);
    m_SkyboxRenderData->copyData(&skyboxData);

    render.setRenderPass(m_SkyboxRenderPass);
    m_SkyboxShader->setDataSlot(0, m_CommonRenderData);
    m_SkyboxShader->setDataSlot(1, m_SkyboxRenderData);
    m_SkyboxShader->setTextureSlot(2, m_SkyboxCubeTexture);
    render.drawItem("world", "box");
    /////////////////// SKY BOX ////////////////////
    ////////////////////////////////////////////////
    render.setRenderPass(m_ScreenTextureRenderPass);
    for (size_t i = 0; i < m_ScreenRenderData.size(); i++) {
        RenderItemData itemData;
        itemData.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 1.0f));
        itemData.model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7 + (i) * 0.45f, -0.7f, 0.0f)) * itemData.model;
        m_ScreenRenderData[i]->copyData(&itemData);

        m_ScreenTextureShader->setDataSlot(0, m_ScreenRenderData[i]);
        m_ScreenTextureShader->setTextureSlot(1, m_DepthMaps[i].texture);
        render.drawItem("world", "plane");
    }
}

void AppLayer::onDetach() {}

void AppLayer::onMouseEvent(Engine::MouseEvent &event) {
    auto &app = Engine::Application::get();

    if (!event.handled && event.type == Engine::EventType::MouseWheel) {
        auto &camera = app.getCamera();
        auto cameraRotation = camera.rotationQuat();

        float deltaX = event.x / 20;
        float deltaY = event.y / 20;

        float eventXSign = deltaX > 0.0f ? 1.0f : -1.0f;
        float eventYSign = deltaY > 0.0f ? 1.0f : -1.0f;

        glm::vec2 mouseOffset =
            glm::vec2(-1.0f * eventXSign * std::abs(deltaX), eventYSign * std::abs(deltaY)) * 0.015f;

        auto deltaRotationX =
            glm::angleAxis(mouseOffset.y, glm::normalize(glm::cross(camera.frontVec(), camera.upVec())));
        auto deltaRotationY = glm::angleAxis(mouseOffset.x, camera.upVec());

        app.getCameraController().rotateTo(deltaRotationX * deltaRotationY * cameraRotation, 0.1);

        m_MousePos = glm::vec2(event.x, event.y);
    }
}

Engine::Mesh AppLayer::createCube2() {
    std::vector<Engine::Vertex> vertices;

    vertices.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(-1.0f, +1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, +1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(-1.0f, -1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(-1.0f, +1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, +1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, -1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f),
                          glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    std::vector<unsigned int> indices = {// front face
                                         0, 1, 2, 0, 2, 3,

                                         // back face
                                         4, 6, 5, 4, 7, 6,

                                         // left face
                                         4, 5, 1, 4, 1, 0,

                                         // right face
                                         3, 2, 6, 3, 6, 7,

                                         // top face
                                         1, 5, 6, 1, 6, 2,

                                         // bottom face
                                         4, 0, 3, 4, 3, 7};

    Engine::Mesh mesh(vertices, indices);
    return mesh;
}

Engine::Mesh AppLayer::createCube(float left, float right, float bottom, float top, float back, float front) {
    // clang-format off
    std::vector<glm::vec3> positions = {
        glm::vec3(-left, top, front), // 0
        glm::vec3(-left, -bottom, front), // 1
        glm::vec3(-left, top, -back), // 2
        glm::vec3(-left, -bottom, -back), // 3
        glm::vec3(right, top, front), // 4
        glm::vec3(right, -bottom, front), // 5
        glm::vec3(right, top, -back), // 6
        glm::vec3(right, -bottom, -back), // 7
    };

    std::vector<Engine::Vertex> vertices;
    vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    
    vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    
    vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    
    vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    // clang-format on

    std::vector<unsigned int> indices(vertices.size());
    std::iota(indices.begin(), indices.end(), 0);

    TBN::calculate(vertices);

    Engine::Mesh mesh(vertices, indices);
    return mesh;
}

Engine::Mesh AppLayer::createPlane(float tileSize, int columns, int rows) {
    // clang-format off
    glm::vec2 center;
    center.x = columns * tileSize / 2.0f;
    center.y = rows * tileSize / 2.0f;

    std::vector<Engine::Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= rows; i++) {
        for (int j = 0; j <= columns; j++) {
            float x = static_cast<float>(j);
            float z = static_cast<float>(i);

            vertices.emplace_back(
                glm::vec3(x * tileSize - center.x, z * tileSize - center.y, 0.0f), 
                glm::vec3(0.0f),
                glm::vec2(x, z),
                glm::vec3(0.0f),
                glm::vec3(0.0f),
                glm::vec3(0.8f, 0.6f, 0.1f)
            );

            if (i != rows && j != columns) {
                unsigned int v1 = (i + 1) * (columns + 1) + j;
                unsigned int v2 = i * (columns + 1) + j + 1;
                unsigned int v3 = i * (columns + 1) + j;

                indices.push_back(v1);
                indices.push_back(v2);
                indices.push_back(v3);

                v1 = (i + 1) * (columns + 1) + j;
                v2 = (i + 1) * (columns + 1) + j + 1;
                v3 = i * (columns + 1) + j + 1;

                indices.push_back(v1);
                indices.push_back(v2);
                indices.push_back(v3);
            }
        }
    }
    // clang-format on

    TBN::calculate(vertices, indices);

    Engine::Mesh mesh(vertices, indices);
    return mesh;
}