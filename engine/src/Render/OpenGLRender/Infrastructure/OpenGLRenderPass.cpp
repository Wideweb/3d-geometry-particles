#include "OpenGLRenderPass.hpp"

#include "OpenGLUtils.hpp"

OpenGLRenderPass::OpenGLRenderPass(
    std::shared_ptr<ShaderProgram> shaderProgram,
    const std::vector<size_t>& constantBufferDescriotions
) {
    m_ShaderProgram = shaderProgram;

    for (size_t i = 0; i < constantBufferDescriotions.size(); i++) {
        GLuint id;
        glGenBuffers(1, &id);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, constantBufferDescriotions[i], NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, i, id); 

        m_ConstantBuffers.push_back(id);
    }

    // glBindBuffer(GL_UNIFORM_BUFFER, uboExampleBlock);
    // glBufferData(GL_UNIFORM_BUFFER, 152, NULL, GL_STATIC_DRAW); // allocate 152 bytes of memory
    // glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLRenderPass::bind() const {
    m_ShaderProgram->bind();

    for (size_t i = 0; i < m_ConstantBuffers.size(); i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, i, m_ConstantBuffers[i]); 
    }
}

void OpenGLRenderPass::unbind() const {
    m_ShaderProgram->unbind();

    for (size_t i = 0; i < m_ConstantBuffers.size(); i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, i, 0); 
    }
}

void OpenGLRenderPass::release() {
    if (m_Resource > 0) {
        glDeleteFramebuffers(1, &m_Resource);
        m_Resource = 0;
    }
}