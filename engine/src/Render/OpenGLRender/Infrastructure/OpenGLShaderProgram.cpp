#pragma once

#include "OpenGLShaderProgram.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

OpenGLShaderProgram::OpenGLShaderProgram(const std::string& vertexFile, const std::string& pixelFile) noexcept {   
    // m_VertexShader = OpenGLUtils::compileShader(vertexFile, nullptr, "VS", "vs_5_1");
    // m_PixelShader = OpenGLUtils::compileShader(pixelFile, nullptr, "PS", "ps_5_1");

    GLuint vertexShader = OpenGLUtils::compileShader(vertexFile, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return;
    }

    GLuint fragmentShader = OpenGLUtils::compileShader(pixelFile, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        return;
    }

    m_ProgramId = glCreateProgram();
    if (m_ProgramId == 0) {
        throw std::runtime_error("Failed to create gl program");
    }

    glAttachShader(m_ProgramId, vertexShader);
    glAttachShader(m_ProgramId, fragmentShader);

    glLinkProgram(m_ProgramId);

    GLint linked_status = 0;
    glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &linked_status);
    if (linked_status == 0) {
        GLint infoLen = 0;
        glGetProgramiv(m_ProgramId, GL_INFO_LOG_LENGTH, &infoLen);

        std::vector<char> infoLog(static_cast<size_t>(infoLen));
        glGetProgramInfoLog(m_ProgramId, infoLen, nullptr, infoLog.data());

        throw std::runtime_error(infoLog.data());
        glDeleteProgram(m_ProgramId);
        return;
    }

    glDetachShader(m_ProgramId, vertexShader);
    glDetachShader(m_ProgramId, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

OpenGLShaderProgram::~OpenGLShaderProgram() {
    release();
}

void OpenGLShaderProgram::bind() const {
    glUseProgram(m_ProgramId);
}

void OpenGLShaderProgram::unbind() const {
    glUseProgram(0);
}
    
void OpenGLShaderProgram::release() {
    if (m_ProgramId > 0) {
        glDeleteProgram(m_ProgramId);
        m_ProgramId = 0;
    }
}