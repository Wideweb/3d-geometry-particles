#include "AppLayer.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include <cmath>

void AppLayer::onAttach() {
    auto& app = Engine::Application::get();
    auto& camera = app.getCamera();

    app.getRender().setClearColor(glm::vec4(1.0f));

    auto vertexSrc = Engine::File::read("./assets/shaders/vertex.glsl");
    auto fragmentSrc = Engine::File::read("./assets/shaders/fill.fragment.glsl");

    m_Shader = Engine::Shader(vertexSrc, fragmentSrc);
    
    m_GeometryModel = Engine::ModelLoader::loadObj("./assets/models/arrow.obj");
    m_GeometryModel->setUp();
    m_GeometryTransform = glm::scale(m_GeometryTransform, glm::vec3(4.0f, 4.0f, 4.0f));

    m_ParticleModel = Engine::ModelLoader::loadObj("./assets/models/bug.obj");
    m_ParticleModel->setUp();
    m_ParticleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));

    camera.setPosition(glm::vec3(8.0f, 6.0f, 8.0f));
    camera.setRotation(glm::quat(glm::vec3(glm::radians(-25.0f), glm::radians(45.0f), 0.0f)));

    for (size_t i = 0; i < 200; i++) {
        m_Particles.emplace_back(m_GeometryModel->meshes[0]);
        m_Particles[m_Particles.size() - 1].setUp();
    }
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

    m_Shader.bind();
    m_Shader.setMatrix4("u_view", camera.viewMatrix());
    m_Shader.setMatrix4("u_projection", camera.projectionMatrix());
    m_Shader.setFloat3("u_lightPos", glm::vec3(8.0f, 4.0f, 4.0f));

    m_GeometryTransform = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(0.0f, 0.0f, 1.0f)) * m_GeometryTransform;
}

void AppLayer::onDraw() { 
    m_Shader.bind();
    m_Shader.setMatrix4("u_model", m_GeometryTransform);
    m_Shader.setFloat4("u_color", glm::vec4(0.25f, 0.75f, 0.1f, 1.0f));
    m_GeometryModel->draw();

    for (auto& particle : m_Particles) {
        m_Shader.setMatrix4("u_model", m_GeometryTransform * particle.getTransform() * m_ParticleTransform);
        m_Shader.setFloat4("u_color", glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
        m_ParticleModel->draw();
    }
}

void AppLayer::onDetach() { }

void AppLayer::onMouseEvent(Engine::MouseEvent &event) {
    auto& app = Engine::Application::get();

    if (!event.handled && event.type == Engine::EventType::MouseWheel) {
        auto mousePos = app.getInput().GetMousePosition();
        auto &camera = app.getCamera();
        auto cameraRotation = camera.rotationQuat();

        float eventXSign = event.x > 0.0f ? 1.0f : -1.0f;
        float eventYSign = event.y > 0.0f ? 1.0f : -1.0f;

        glm::vec2 mouseOffset =
            glm::vec2(-1.0f * eventXSign * std::pow(event.x, 2), eventYSign * std::pow(event.y, 2)) * 0.015f;

        auto deltaRotationX =
            glm::angleAxis(mouseOffset.y, glm::normalize(glm::cross(camera.frontVec(), camera.upVec())));
        auto deltaRotationY = glm::angleAxis(mouseOffset.x, camera.upVec());

       app.getCameraController().rotateTo(deltaRotationX * deltaRotationY * cameraRotation, 0.1);
    }
}
