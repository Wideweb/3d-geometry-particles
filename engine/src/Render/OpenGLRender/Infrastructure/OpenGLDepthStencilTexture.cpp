#include <stdexcept>

#include "RenderTexture.hpp"

namespace Engine {

OpenGLDepthStencilTexture::OpenGLDepthStencilTexture(size_t width, size_t height) {
    m_Format   = GL_DEPTH24_STENCIL8;
    m_DataType = GL_UNSIGNED_INT_24_8;
    m_Width    = width;
    m_Height   = height;

    glGenTextures(1, &m_Resource);
    glBindTexture(GL_TEXTURE_2D, m_Resource);

    glTexImage2D(gl.TEXTURE_2D, 0, m_Format, m_Width, m_Height, 0, m_Format, m_DataType, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindTexture(GL_TEXTURE_2D, 0);

    setClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

OpenGLDepthStencilTexture::~OpenGLDepthStencilTexture() { release(); }

void OpenGLDepthStencilTexture::resize(size_t width, size_t height) {
    if (width == m_Width && height == m_Height) {
        return;
    }

    if (width > UINT32_MAX || height > UINT32_MAX) {
        throw std::out_of_range("Invalid width/height");
    }

    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, m_Format, width, height, 0, m_Format, m_DataType, NULL);
    unbind();

    m_Width  = width;
    m_Height = height;
}

void OpenGLDepthStencilTexture::bind() const { glBindTexture(GL_TEXTURE_2D, m_Resource); }

void OpenGLDepthStencilTexture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

void OpenGLDepthStencilTexture::release() {
    if (m_Resource > 0) {
        glDeleteTextures(1, &m_Resource);
    }
}

}  // namespace Engine