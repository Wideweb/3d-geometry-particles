#pragma once

#include "DxUtils.hpp"
#include "Framebuffer.hpp"
#include "ShaderProgram.hpp"

#include <memory>
#include <vector>

class OpenGLRenderPass {
public:
    OpenGLRenderPass(std::shared_ptr<OpenGLRenderPass> shaderProgram, const std::vector<size_t>& constantBufferDescriotions);

    void bind() const;
    void unbind() const;

    void release();

private:
    std::vector<GLuint>                               m_ConstantBuffers;
    std::shared_ptr<OpenGLShaderProgram>              m_ShaderProgram;
};