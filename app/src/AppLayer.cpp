#include "AppLayer.hpp"

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <unordered_map>

void AppLayer::onAttach() {
    auto& app    = Engine::Application::get();
    auto& camera = app.getCamera();
    auto& render = app.getRender();

    camera.setPosition(glm::vec3(-24.0f, 12.0f, 24.0f));
    camera.setRotation(glm::quat(glm::vec3(glm::radians(-25.0f), glm::radians(-45.0f), 0.0f)));

    render.beginInitialization();

    m_CommonDataBuffer = render.createShaderProgramDataBuffer(sizeof(GfxEffect::RenderCommonData));

    m_Effects.push_back(std::make_shared<GeometryParticleEffect>());
    m_Effects.push_back(std::make_shared<CascadeShadowEffect>());
    m_Effects.push_back(std::make_shared<SkyboxEffect>());

    for (auto& effect : m_Effects) {
        effect->bind();
    }

    render.endInitialization();
}

void AppLayer::onUpdate() {
    auto& app              = Engine::Application::get();
    auto& camera           = app.getCamera();
    auto& time             = app.getTime();
    auto& input            = app.getInput();
    auto& cameraController = app.getCameraController();

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

    glm::vec3 lightDir  = glm::normalize(glm::vec3(-1.0f, -5.0f, -5.0f));
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    m_CommonData.view            = glm::transpose(camera.viewMatrix());
    m_CommonData.projection      = glm::transpose(camera.projectionMatrix());
    m_CommonData.viewPos         = camera.positionVec();
    m_CommonData.ambientLight    = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_CommonData.light.view      = lightView;
    m_CommonData.light.direction = lightDir;
    m_CommonData.light.strength  = glm::vec3(1.0f);
    m_CommonData.time            = time.getTotalSeconds();

    for (auto& effect : m_Effects) {
        effect->update(m_CommonData);
    }
}

void AppLayer::onDraw() {
    m_CommonDataBuffer->copyData(&m_CommonData);

    for (auto& effect : m_Effects) {
        effect->draw(m_CommonDataBuffer);
    }
}

void AppLayer::onDetach() {}

void AppLayer::onMouseEvent(Engine::MouseEvent& event) {
    auto& app = Engine::Application::get();

    if (!event.handled && event.type == Engine::EventType::MouseWheel) {
        auto& camera         = app.getCamera();
        auto  cameraRotation = camera.rotationQuat();

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