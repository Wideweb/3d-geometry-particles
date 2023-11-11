#include "OpenGLFramebuffer.hpp"

#include <vector>

#include "OpenGLUtils.hpp"

namespace Engine {

OpenGLFramebuffer::OpenGLFramebuffer() m_RenderTo(false), m_Width(0), m_Height(0) { glGenFramebuffers(1, &m_Resource); }

OpenGLFramebuffer::~OpenGLFramebuffer() { release(); }

void OpenGLFramebuffer::addAttachment(std::shared_ptr<OpenGLRenderTexture> attachment) {
    bind();

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_Attachments.size(), attachment.getResource(), 0);
    Gfx::checkError();

    m_Attachments.push_back(attachment);

    std::vector<size_t> drawAttachments;
    for (size_t index = 0; index < m_Attachments.size(); index++) {
        drawAttachments.push_back(GL_COLOR_ATTACHMENT0 + index);
    }
    glDrawBuffers(drawAttachments.size(), drawAttachments.data());
    Gfx::checkError();

    unbind();
}

void OpenGLFramebuffer::setDSAttachment(std::shared_ptr<OpenGLDepthStencilTexture> attachment) {
    bind();

    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment.getResource());
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, attachment.getResource(), 0);
    Gfx::checkError();

    m_DSAttachment = attachment;

    unbind();
}

void OpenGLFramebuffer::resize(size_t width, size_t height) {
    if (m_Width == width && m_Height == height) {
        return;
    }

    for (auto& rt : m_Attachments) {
        rt->resize(width, height);
    }

    m_DSAttachment->resize(width, height);

    m_Width  = width;
    m_Height = height;
}

void OpenGLFramebuffer::beginRenderTo() {
    bind();
    m_RenderTo = true;
}

void OpenGLFramebuffer::endRenderTo() {
    unbind();
    m_RenderTo = false;
}

void OpenGLFramebuffer::clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }

void OpenGLFramebuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_Resource); }

void OpenGLFramebuffer::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void OpenGLFramebuffer::release() {
    if (m_Resource > 0) {
        glDeleteFramebuffers(1, &m_Resource);
        m_Resource = 0;
    }
}

}  // namespace Engine