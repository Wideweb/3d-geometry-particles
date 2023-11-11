#pragma once

#include "glad/glad.h"

namespace Engine {

class OpenGLRenderTexture {
public:
    OpenGLRenderTexture(GLenum format, GLenum dataType, size_t width, size_t height);

    virtual ~OpenGLRenderTexture();

    void resize(size_t width, size_t height);

    void release();

    void setClearColor(float r, float g, float b, float a) {
        m_ClearColor[0] = r;
        m_ClearColor[1] = g;
        m_ClearColor[2] = b;
        m_ClearColor[3] = a;
    }

    GLuint getResource() const noexcept { return m_Resource; }

    GLenum getFormat() const noexcept { return m_Format; }
    GLenum getDataType() const noexcept { return m_DataType; }

    const float* getClearColor() const noexcept { return m_ClearColor; }

    void bind() const;
    void unbind() const;

private:
    GLuint m_Resource;
    float  m_ClearColor[4];

    GLenum m_Format;
    GLenum m_DataType;

    size_t m_Width;
    size_t m_Height;
};

}  // namespace Engine