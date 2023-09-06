#pragma once

#include "Layer.hpp"
#include "MasterRenderer.hpp"
#include "Window.hpp"

#include <string>

namespace Engine {

class Layer {
  protected:
    std::string m_Name;
    bool m_Active = false;

  public:
    explicit Layer(std::string name);
    virtual ~Layer() = default;

    bool isActive() { return m_Active; }

    void attach();
    void update();
    void draw();
    void detach();

    std::string &getName() { return m_Name; }

    virtual void onAttach() {}
    virtual void onUpdate() {}
    virtual void onDraw() {}
    virtual void onDetach() {}

    virtual void onMouseEvent(MouseEvent &) {}
    virtual void onWindowEvent(WindowEvent &) {}
};

} // namespace Engine