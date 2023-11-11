#include "OpenGLRenderPass.hpp"

#include "OpenGLUtils.hpp"

namespace Engine {

OpenGLRenderPass::OpenGLRenderPass(std::shared_ptr<ShaderProgram> shaderProgram) { m_ShaderProgram = shaderProgram; }

void OpenGLRenderPass::bind() const { m_ShaderProgram->bind(); }

void OpenGLRenderPass::unbind() const { m_ShaderProgram->unbind(); }

}  // namespace Engine