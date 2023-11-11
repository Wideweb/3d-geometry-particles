#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <memory>

#include "Camera.hpp"
#include "Framebuffer.hpp"
#include "Viewport.hpp"

namespace Engine {

class MasterRenderer {
private:
    Viewport    m_Viewport;
    Framebuffer m_Framebuffer;
    glm::vec4   m_ClearColor = glm::vec4(0.0f);

public:
    MasterRenderer(unsigned int width, unsigned int height);
    ~MasterRenderer();

    void            begin();
    void            end();
    void            setClearColor(glm::vec4 color);
    glm::vec4       getClearColor();
    void            setViewport(int width, int height);
    const Viewport& getViewport();
    void            setFramebuffer(Framebuffer& framebuffer);
    Framebuffer&    getFramebuffer() { return m_Framebuffer; }
    void            clear();
};

}  // namespace Engine