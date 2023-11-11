#include "MasterRenderer.hpp"

#include "glad/glad.h"

namespace Engine {

MasterRenderer::MasterRenderer(unsigned int width, unsigned int height)
: m_Viewport{width, height},
  m_Framebuffer(Framebuffer::createDefault()) {}

void MasterRenderer::begin() {
    m_Viewport.use();
    m_Framebuffer.bind();
}

void MasterRenderer::end() { m_Framebuffer.unbind(); }

void MasterRenderer::setClearColor(glm::vec4 color) {
    glClearColor(color.r, color.g, color.b, color.a);
    m_ClearColor = color;
}

glm::vec4 MasterRenderer::getClearColor() { return m_ClearColor; }

void MasterRenderer::setViewport(int width, int height) {
    m_Viewport.width  = width;
    m_Viewport.height = height;
}

const Viewport& MasterRenderer::getViewport() { return m_Viewport; }

void MasterRenderer::setFramebuffer(Framebuffer& framebuffer) { m_Framebuffer = framebuffer; }

void MasterRenderer::clear() {
    m_Framebuffer.bind();
    m_Framebuffer.clear();
    m_Framebuffer.unbind();
}

MasterRenderer::~MasterRenderer() {}

}  // namespace Engine