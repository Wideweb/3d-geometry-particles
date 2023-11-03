#include "AppLayer.hpp"

#include "TBN.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <unordered_map>
#include <algorithm>

void AppLayer::onAttach() {
    auto& app = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& render = app.getRender();

    render.beginInitialization();

    m_Texture = render.loadTexture("./../assets/box-2.png");

    Engine::Mesh bug = Engine::ModelLoader::loadObj("./../assets/models/bug.obj");
    Engine::Mesh monkey = Engine::ModelLoader::loadObj("./../assets/models/monkey.obj");

    render.registerGeometry("world", {"monkey", "bug"}, {monkey, bug});

    std::vector<Engine::ShaderProgramSlotDesc> slotsTexturePass =
    {
        { "cbCommon",    Engine::SHADER_PROGRAM_SLOT_TYPE::DATA },
        { "cbObject",    Engine::SHADER_PROGRAM_SLOT_TYPE::DATA },
        { "diffuseMap",  Engine::SHADER_PROGRAM_SLOT_TYPE::TEXTURE },
    };

    m_ShaderTexture = render.createShaderProgram("./../assets/shaders/dx/texture.hlsl", "./../assets/shaders/dx/texture.hlsl", slotsTexturePass);
    m_RenderPassTexture = render.createRenderPass(m_ShaderTexture);

    std::vector<Engine::ShaderProgramSlotDesc> slotsColorPass =
    {
        { "cbCommon",    Engine::SHADER_PROGRAM_SLOT_TYPE::DATA },
        { "cbObject",    Engine::SHADER_PROGRAM_SLOT_TYPE::DATA },
    };

    m_ShaderColor = render.createShaderProgram("./../assets/shaders/dx/color.hlsl", "./../assets/shaders/dx/color.hlsl", slotsColorPass);
    m_RenderPassColor = render.createRenderPass(m_ShaderColor);

    m_CommonRenderData = render.createShaderProgramDataBuffer(sizeof(RenderCommonData));
    m_GeometryRenderData = render.createShaderProgramDataBuffer(sizeof(RenderCommonData));

    // app.getRender().setClearColor(glm::vec4(1.0f));

    // auto vertexSrc = Engine::File::read("./assets/shaders/vertex.glsl");
    // auto fragmentSrc = Engine::File::read("./assets/shaders/fill.fragment.glsl");
    // m_Shader = Engine::Shader(vertexSrc, fragmentSrc);
    
    // m_GeometryModel = Engine::ModelLoader::loadObj("./assets/models/monkey.obj");
    // m_GeometryModel->setUp();
    // m_GeometryTransform = glm::scale(m_GeometryTransform, glm::vec3(5.0f, 5.0f, 5.0f));
    // // m_GeometryTransform = glm::rotate(glm::mat4(1.0f), -3.14f, glm::vec3(0.0f, 0.0f, 1.0f)) * m_GeometryTransform;
    // m_GeometryTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f)) * m_GeometryTransform;
    // // m_GeometryTransform = glm::rotate(glm::mat4(1.0f), -0.25f, glm::vec3(1.0f, 0.0f, 0.0f)) * m_GeometryTransform;

    // m_ParticleModel = Engine::ModelLoader::loadObj("./assets/models/bug.obj");
    // m_ParticleModel->setUp();
    m_ParticleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));

    // m_SkyTexture = Engine::TextureLoader::loadTexture("./assets/sky-3.jpeg");
    // m_SandTexture = Engine::TextureLoader::loadTexture("./assets/sand-2.png");

    // vertexSrc = Engine::File::read("./assets/shaders/vertex.glsl");
    // fragmentSrc = Engine::File::read("./assets/shaders/water.fragment.glsl");
    // m_SurfaceShader = Engine::Shader(vertexSrc, fragmentSrc);

    // m_SurfaceModel = Engine::ModelFactory::createPlane(4, 10, 10);
    // m_SurfaceModel->setUp();

    camera.setPosition(glm::vec3(1.0f, 0.5f, 1.0f));
    camera.setRotation(glm::quat(glm::vec3(glm::radians(-25.0f), glm::radians(45.0f), 0.0f)));

    m_Geometry = std::make_shared<Geometry>(monkey);
    m_Particles.reserve(100);
    for (size_t i = 0; i < 100; i++) {
        m_Particles.emplace_back(*m_Geometry);
        m_Particles[i].setUp();
        m_ParticlesRenderData.push_back(render.createShaderProgramDataBuffer(sizeof(RenderItemDataColorPass)));
    }

    render.endInitialization();
 }

void AppLayer::onUpdate() { 
    auto& app = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& time = app.getTime();
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

    for (auto& particle : m_Particles) {
        particle.update();
    }

    // m_Shader.bind();
    // m_Shader.setMatrix4("u_view", camera.viewMatrix());
    // m_Shader.setMatrix4("u_projection", camera.projectionMatrix());
    // m_Shader.setFloat3("u_lightPos", glm::vec3(8.0f, 4.0f, 4.0f));

    // m_SurfaceShader.bind();
    // m_SurfaceShader.setTexture("u_refraction", m_SandTexture);
    // m_SurfaceShader.setTexture("u_reflection", m_SkyTexture);
    // m_SurfaceShader.setMatrix4("u_view", camera.viewMatrix());
    // m_SurfaceShader.setMatrix4("u_projection", camera.projectionMatrix());
    // m_SurfaceShader.setFloat3("u_lightPos", glm::vec3(8.0f, 8.0f, 4.0f));
    // m_SurfaceShader.setFloat("u_time", m_Time);
    // m_SurfaceShader.setFloat3("u_viewPos", camera.positionVec());
    m_Time += 0.016f;

    // m_GeometryTransform = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(0.0f, 0.0f, 1.0f)) * m_GeometryTransform;
}

void AppLayer::onDraw() { 
    auto &app = Engine::Application::get();
    auto &camera = app.getCamera();
    auto &render = app.getRender();
    auto &time = app.getTime();

    m_WorldTransform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
    // m_WorldTransform = glm::rotate(glm::mat4(1.0f), (float)std::sin(time.getTotalSeconds()), glm::vec3(1.0f, 0.0f, 0.0f)) * m_WorldTransform;
    // m_WorldTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * m_WorldTransform;
    // m_WorldTransform = glm::transpose(m_WorldTransform);

    render.setRenderPass(m_RenderPassTexture);

    RenderCommonData commonData;
    commonData.view = glm::transpose(camera.viewMatrix());
    commonData.projection = glm::transpose(camera.projectionMatrix());
    m_CommonRenderData->copyData(&commonData);
    
    m_ShaderTexture->setDataSlot(0, m_CommonRenderData);

    RenderItemDataTexturePass itemData;
    itemData.model = glm::transpose(m_WorldTransform);
    m_GeometryRenderData->copyData(&itemData);

    m_ShaderTexture->setDataSlot(1, m_GeometryRenderData);
    m_ShaderTexture->setTextureSlot(2, m_Texture);

    render.drawItem("world", "monkey");

    // m_WorldTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * m_WorldTransform;
    // itemData.model = glm::transpose(m_WorldTransform);
    // m_GeometryRenderData2->copyData(&itemData);
    // m_Shader->setDataSlot(1, m_GeometryRenderData2);
    // render.drawItem("world", "cube");


    // m_SurfaceShader.bind();
    // m_SurfaceShader.setMatrix4("u_model", m_SurfaceTransform);
    // m_SurfaceShader.setFloat4("u_color", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
    // m_SurfaceModel->draw();

    // m_Shader.bind();
    // m_Shader.setMatrix4("u_model", m_GeometryTransform);
    // m_Shader.setFloat4("u_color", glm::vec4(0.25f, 0.75f, 0.1f, 1.0f));
    // m_GeometryModel->draw();

    render.setRenderPass(m_RenderPassColor);

    m_ShaderColor->setDataSlot(0, m_CommonRenderData);
    for (size_t i = 0; i < m_Particles.size(); i++) {
        RenderItemDataColorPass itemData;
        itemData.model = glm::transpose(m_WorldTransform * m_Particles[i].getTransform() * m_ParticleTransform);
        itemData.color = glm::vec4(0.25f, 0.75f, 0.1f, 1.0f);
        m_ParticlesRenderData[i]->copyData(&itemData);
        m_ShaderColor->setDataSlot(1, m_ParticlesRenderData[i]);
        render.drawItem("world", "bug");
    }
}

void AppLayer::onDetach() { }

void AppLayer::onMouseEvent(Engine::MouseEvent &event) {
    auto& app = Engine::Application::get();

    if (!event.handled && event.type == Engine::EventType::MouseWheel) {
        auto &camera = app.getCamera();
        auto cameraRotation = camera.rotationQuat();

        float deltaX = event.x / 20;
        float deltaY = event.y / 20;

        float eventXSign = deltaX > 0.0f ? 1.0f : -1.0f;
        float eventYSign = deltaY > 0.0f ? 1.0f : -1.0f;

        glm::vec2 mouseOffset = glm::vec2(-1.0f * eventXSign * std::abs(deltaX), eventYSign * std::abs(deltaY)) * 0.015f;

        auto deltaRotationX = glm::angleAxis(mouseOffset.y, glm::normalize(glm::cross(camera.frontVec(), camera.upVec())));
        auto deltaRotationY = glm::angleAxis(mouseOffset.x, camera.upVec());

       app.getCameraController().rotateTo(deltaRotationX * deltaRotationY * cameraRotation, 0.1);

       m_MousePos = glm::vec2(event.x, event.y);
    }
}

Engine::Mesh AppLayer::createCube2() {
    std::vector<Engine::Vertex> vertices;

    vertices.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(-1.0f, +1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, +1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(-1.0f, -1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(-1.0f, +1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, +1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
    vertices.emplace_back(glm::vec3(+1.0f, -1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

    std::vector<unsigned int> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

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