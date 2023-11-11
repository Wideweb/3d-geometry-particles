#pragma once

#include "OpenGLUtils.hpp"

namespace Engine {

class OpenGLShaderProgramSlot {
public:
    OpenGLShaderProgramSlot(size_t byteSize);

    OpenGLShaderProgramSlot(const OpenGLShaderProgramSlot& rhs)            = delete;
    OpenGLShaderProgramSlot& operator=(const OpenGLShaderProgramSlot& rhs) = delete;

    ~OpenGLShaderProgramSlot();

    void copyData(void* data);
    void bind() const;
    void unbind() const;
    void release();

    GLuint getResource() const noexcept { return m_Resource; }

private:
    GLuint m_Resource;
    size_t m_ByteSize;
};

class OpenGLShaderProgramTextureSlot {
public:
    GLuint resource;
    GLuint location;
};

}  // namespace Engine