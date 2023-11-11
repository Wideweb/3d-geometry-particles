#include "Application.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "Math.hpp"

namespace Engine {

Application* Application::s_Instance = nullptr;

Application::Application(void* appInstance) {
    WindowProps windowProps{.width = 960, .height = 540, .antialiasing = true, .appInstance = appInstance};
    m_Window = std::unique_ptr<Window>(Window::create(windowProps));
    m_Window->setMouseEventCallback(std::bind(&Application::onMouseEvent, this, std::placeholders::_1));
    m_Window->setWindowEventCallback(std::bind(&Application::onWindowEvent, this, std::placeholders::_1));

    m_Input = std::unique_ptr<Input>(Input::create());

    m_Render = std::unique_ptr<CrossPlatformRender>(
        CrossPlatformRender::create(m_Window->getNaviteWindow(), windowProps.width, windowProps.height)
    );

    m_Camera =
        std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0, 0.0f, -1.0f));
    m_Camera->setSize(960, 540);
    m_Camera->setPerspective(glm::radians(45.0f), 0.1f, 200.0f);
    m_Camera->setProjection(Camera::Projection::PERSPECTIVE);

    m_CameraController = std::make_unique<CameraController>(*m_Camera);

    s_Instance = this;
}

void Application::run() {
    Math::srand();

    m_Time.tick();

    while (m_Running) {
        m_Time.tick();

        m_Input->update();
        m_CameraController->update(m_Time.getDeltaSeconds());

        for (auto layer : m_LayerStack) {
            layer->update();

            if (!layer->isActive()) {
                break;
            }
        }

        m_Render->beginFrame();
        m_Render->clear(0.25f, 0.6f, 0.6f, 1.0f);
        for (auto layer : m_LayerStack) {
            layer->draw();
        }
        m_Render->endFrame();

        m_Window->swapBuffers();
        m_Window->readInput();
    }
}

void Application::stop() { m_Running = false; }

void Application::onMouseEvent(MouseEvent& e) {
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
        (*it)->onMouseEvent(e);
        if (e.handled) {
            break;
        }
    }
}

void Application::onWindowEvent(WindowEvent& e) {
    if (e.type == EventType::WindowResized) {
        m_Render->resize(m_Window->getWidth(), m_Window->getHeight());
        m_Camera->setSize(m_Window->getWidth(), m_Window->getHeight());
    }

    for (auto layer : m_LayerStack) {
        layer->onWindowEvent(e);
    }

    if (e.type == EventType::WindowClosed) {
        stop();
        m_Render->release();
    }
}

Application::~Application() {
    for (auto layer : m_LayerStack) {
        layer->onDetach();
    }
    m_Window->shutDown();
}

}  // namespace Engine
