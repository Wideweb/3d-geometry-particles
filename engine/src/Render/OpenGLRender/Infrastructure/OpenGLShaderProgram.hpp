#pragma once

#include "OpenGLUtils.hpp"

#include <string>

class OpenGLShaderProgram {
public:
    OpenGLShaderProgram(const std::string& vertexFile, const std::string& pixelFile) noexcept;

    ~OpenGLShaderProgram();
    
    void release();

    void bind() const;
    void unbind() const;

private:
    GLuint m_ProgramId;
};