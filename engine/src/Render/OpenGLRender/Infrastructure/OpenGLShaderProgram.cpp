#pragma once

#include "OpenGLShaderProgram.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

namespace Engine {

OpenGLShaderProgram::OpenGLShaderProgram(
    const std::string& vertexFile, const std::string& pixelFile, const std::vector<size_t>& dataSlots,
    const std::vector<std::string>& textureSlots
) noexcept {
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

    m_DataSlots.reserve(dataSlots.size());
    for (size_t i = 0; i < dataSlots.size(); i++) {
        m_DataSlots.push_back(std::make_unique<OpenGLShaderProgramSlot>(dataSlots[i]));
    }

    m_TextureSlots.reserve(textureSlots.size());
    for (size_t i = 0; i < textureSlots.size(); i++) {
        GLint location = glGetUniformLocation(m_ProgramId, textureSlots[i].c_str());
        m_TextureSlots.emplace_back(-1, location);
    }
}

OpenGLShaderProgram::~OpenGLShaderProgram() { release(); }

void OpenGLShaderProgram::setDataSlot(size_t index, void* data) { m_DataSlots[index]->copyData(data); }

void OpenGLShaderProgram::setTextureSlot(size_t index, GLuint resource) { m_TextureSlots[index].resource = resource; }

void OpenGLShaderProgram::setTextureSlot(size_t index, std::shared_ptr<OpenGLRenderTexture> renderTexture) {
    setTextureSlot(index, renderTexture->getResource());
}

void OpenGLShaderProgram::bind() const {
    glUseProgram(m_ProgramId);

    for (size_t i = 0; i < m_DataSlots.size(); i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, i, m_DataSlots[i]->getResource());
    }

    for (size_t i = 0; i < m_TextureSlots.size(); i++) {
        if (m_TextureSlots[i].resource > 0) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_TextureSlots[i].resource);
            glUniform1i(m_TextureSlots[i].location, i);
        }
    }
}

void OpenGLShaderProgram::unbind() const { glUseProgram(0); }

void OpenGLShaderProgram::release() {
    if (m_ProgramId > 0) {
        glDeleteProgram(m_ProgramId);
        m_ProgramId = 0;
    }
}

}  // namespace Engine