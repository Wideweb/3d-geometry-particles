#pragma once

#include "OpenGLDepthStencilTexture.hpp"
#include "OpenGLOpenGLRenderTexture.hpp"
#include "glad/glad.h"

namespace Engine {

class OpenGLFramebuffer {
public:
    OpenGLFramebuffer();

    virtual ~OpenGLFramebuffer();

    void addAttachment(std::shared_ptr<OpenGLRenderTexture> attachment);

    void setDSAttachment(std::shared_ptr<OpenGLDepthStencilTexture> attachment);

    void resize(size_t width, size_t height);

    void beginRenderTo();

    void endRenderTo();

    void clear();

    const std::vector<std::shared_ptr<OpenGLRenderTexture>>& getAttachments() const noexcept { return m_Attachments; }

    std::shared_ptr<OpenGLDepthStencilTexture> getDSAttachment() const noexcept { return m_DSAttachment; }

    void bind() const;
    void unbind() const;

    void release();

private:
    GLuint m_Resource;

    size_t m_Width;
    size_t m_Height;

    std::vector<std::shared_ptr<OpenGLRenderTexture>> m_Attachments;
    std::shared_ptr<OpenGLDepthStencilTexture>        m_DSAttachment;

    bool m_RenderTo;
};

}  // namespace Engine