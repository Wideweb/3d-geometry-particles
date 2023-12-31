#include "Application.hpp"

#include "Math.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

namespace Engine {

Application *Application::s_Instance = nullptr;

Application::Application() {
    WindowProps windowProps{.width = 960, .height = 540, .antialiasing = true};
    m_Window = std::unique_ptr<Window>(Window::create(windowProps));
    m_Window->setMouseEventCallback(std::bind(&Application::onMouseEvent, this, std::placeholders::_1));
    m_Window->setWindowEventCallback(std::bind(&Application::onWindowEvent, this, std::placeholders::_1));

    m_Input = std::unique_ptr<Input>(Input::create());

    m_Render = std::make_unique<MasterRenderer>(960, 540);
    m_Render->setClearColor({0.25f, 0.6f, 0.6f, 1.0f});

    m_Camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0, 0.0f, -1.0f));
    m_Camera->setSize(960, 540);
    m_Camera->setPerspective(glm::radians(45.0f), 0.1f, 50.0f);
    m_Camera->setProjection(Camera::Projection::PERSPECTIVE);

    m_CameraController = std::make_unique<CameraController>(*m_Camera);

    s_Instance = this;
}

void Application::run() {
    Math::srand();
    
    m_Time.tick();

    while (m_Running) {
        m_Time.tick();

        m_Window->readInput();
        m_Input->update();
        m_CameraController->update(m_Time.getDeltaSeconds());

        for (auto layer : m_LayerStack) {
            layer->update();

            if (!layer->isActive()) {
                break;
            }
        }

        m_Render->clear();
        m_Render->begin();
        for (auto layer : m_LayerStack) {
            
            layer->draw();
        }
        m_Render->end();

        m_Window->swapBuffers();
    }
}

void Application::stop() { m_Running = false; }

void Application::onMouseEvent(MouseEvent &e) {
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
        (*it)->onMouseEvent(e);
        if (e.handled) {
            break;
        }
    }
}

void Application::onWindowEvent(WindowEvent &e) {
    if (e.type == EventType::WindowResized) {
        m_Render->setViewport(m_Window->getWidth(), m_Window->getHeight());
        m_Camera->setSize(m_Window->getWidth(), m_Window->getHeight());
    }

    for (auto layer : m_LayerStack) {
        layer->onWindowEvent(e);
    }

    if (e.type == EventType::WindowClosed) {
        stop();
    }
}

Application::~Application() {
    for (auto layer : m_LayerStack) {
        layer->onDetach();
    }
    m_Window->shutDown();
}

} // namespace Engine
