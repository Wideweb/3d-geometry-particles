#pragma once

#include <memory>

#include "DxUtils.hpp"
#include "ShaderProgram.hpp"

namespace Engine {

class OpenGLRenderPass {
public:
    OpenGLRenderPass(std::shared_ptr<OpenGLRenderPass> shaderProgram);

    void bind() const;
    void unbind() const;

private:
    std::shared_ptr<OpenGLShaderProgram> m_ShaderProgram;
};

}  // namespace Engine