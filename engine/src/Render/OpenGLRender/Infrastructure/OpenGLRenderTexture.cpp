#include <stdexcept>

#include "RenderTexture.hpp"

namespace Engine {

OpenGLRenderTexture::OpenGLRenderTexture(GLenum format, GLenum dataType, size_t width, size_t height) {
    m_Format   = format;
    m_DataType = dataType;
    m_Width    = width;
    m_Height   = height;

    glGenTextures(1, &m_Resource);
    glBindTexture(GL_TEXTURE_2D, m_Resource);

    glTexImage2D(GL_TEXTURE_2D, 0, m_Format, m_Width, m_Height, 0, m_Format, m_DataType, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    setClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

OpenGLRenderTexture::~OpenGLRenderTexture() { release(); }

void OpenGLRenderTexture::resize(size_t width, size_t height) {
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

void OpenGLRenderTexture::release() {
    if (m_Resource > 0) {
        glDeleteTextures(1, &m_Resource);
        m_Resource = 0;
    }
}

void OpenGLRenderTexture::bind() const { glBindTexture(GL_TEXTURE_2D, m_Resource); }

void OpenGLRenderTexture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

}  // namespace Engine