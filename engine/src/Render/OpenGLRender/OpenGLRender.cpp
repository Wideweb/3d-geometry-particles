#include "OpenGLRender.hpp"

namespace Engine {

OpenGLRender::OpenGLRender(void* window, uint32_t width, uint32_t height)
: m_Window(window),
  m_Width(width),
  m_Height(height) {
    m_GeometryRegistry = std::make_unique<OpenGLGeometryRegistry>();
}

OpenGLRender::~OpenGLRender() {}

void OpenGLRender::beginInitialization() {}

void OpenGLRender::endInitialization() {}

void OpenGLRender::beginFrame() {}

void OpenGLRender::endFrame() {}

void OpenGLRender::resize(uint32_t width, uint32_t height) {
    if (m_Width == width && m_Height == height) {
        return;
    }

    glViewport(0, 0, width, height);

    m_Width  = width;
    m_Height = height;
}

void OpenGLRender::setPass(std::shared_ptr<OpenGLRenderPass> pass) {
    if (m_RenderPass != nullptr) {
        m_RenderPass->unbind();
    }

    if (pass != nullptr) {
        pass->bind();
    }

    m_RenderPass = pass;
}

void OpenGLRender::setFramebuffer(std::shared_ptr<OpenGLFramebuffer> fb) {
    if (m_Framebuffer != nullptr) {
        m_Framebuffer->endRenderTo();
    }

    if (fb == nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        m_Framebuffer->beginRenderTo();
    }

    m_Framebuffer = fb;
}

void OpenGLRender::clear(float r, float g, float b, float a) { glClearColor(r, g, b, a); }

void OpenGLRender::registerGeometry(
    const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
) {
    m_GeometryRegistry->add(geometry, subGeometries, subMeshes);
}

void OpenGLRender::drawItem(const std::string& geometry, const std::string& subGeometry) {
    OpenGLSubmeshGeometry* geo    = m_GeometryRegistry->get(geometry);
    OpenGLMeshGeometry&    subGeo = geo->drawArgs[subGeometry];

    glBindVertexArray(geo->VAO);
    glDrawElementsBaseVertex(
        GL_TRIANGLES, subGeo.indexCount, GL_UNSIGNED_INT, subGeo.startIndexLocation, subGeo.baseVertexLocation
    );
    glBindVertexArray(0);
}

}  // namespace Engine
