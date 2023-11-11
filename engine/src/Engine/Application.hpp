#pragma once

#include <iterator>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Camera.hpp"
#include "CameraController.hpp"
#include "CrossPlatformRender.hpp"
#include "Input.hpp"
#include "Layer.hpp"
#include "Time.hpp"
#include "Window.hpp"

namespace Engine {

class Application {
private:
    static Application* s_Instance;

    std::unique_ptr<Window>                                                      m_Window;
    std::unique_ptr<Input>                                                       m_Input;
    std::unique_ptr<CrossPlatformRender>                                         m_Render;
    std::unique_ptr<Camera>                                                      m_Camera;
    std::unique_ptr<CameraController>                                            m_CameraController;
    std::list<std::shared_ptr<Layer>>                                            m_LayerStack;
    std::unordered_map<std::string, std::list<std::shared_ptr<Layer>>::iterator> m_NameToLayer;
    Time                                                                         m_Time;

    bool m_Running = true;

public:
    Application(void* appInstance);
    virtual ~Application();

    void onMouseEvent(MouseEvent& e);
    void onWindowEvent(WindowEvent& e);

    void initialize();
    void run();
    void stop();

    template <typename T>
    T& addLayer(const std::string& label) {
        auto layer = std::make_shared<T>(label);
        m_LayerStack.push_back(layer);
        m_NameToLayer[label] = std::prev(m_LayerStack.end());
        layer->attach();
        return *layer;
    }

    void removeLayer(const std::string& label) {
        auto layerIt = m_NameToLayer[label];
        (*layerIt)->detach();
        m_LayerStack.erase(layerIt);
        m_NameToLayer.erase(label);
    }

    template <typename T>
    void reloadLayer(const std::string& label) {
        removeLayer(label);
        addLayer<T>(label);
    }

    Window&              getWindow() { return *m_Window; }
    Input&               getInput() { return *m_Input; }
    CrossPlatformRender& getRender() { return *m_Render; }
    Camera&              getCamera() { return *m_Camera; }
    CameraController&    getCameraController() { return *m_CameraController; }
    Time&                getTime() { return m_Time; }
    Layer&               getLayer(const std::string& label) { return **m_NameToLayer[label]; }

    static Application& get() { return *s_Instance; }
};

}  // namespace Engine
