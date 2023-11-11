#pragma once

#include <memory>
#include <string>
#include <vector>

#include "OpenGLRenderTexture.hpp"
#include "OpenGLShaderProgramSlot.hpp"
#include "OpenGLUtils.hpp"

namespace Engine {

class OpenGLShaderProgram {
public:
    OpenGLShaderProgram(
        const std::string& vertexFile, const std::string& pixelFile, const std::vector<size_t>& dataSlots,
        const std::vector<std::string>& textureSlots
    ) noexcept;

    ~OpenGLShaderProgram();

    void release();

    void setDataSlot(size_t index, void* data);
    void setTextureSlot(size_t index, GLuint resource);
    void setTextureSlot(size_t index, std::shared_ptr<OpenGLRenderTexture> renderTexture);

    void bind() const;
    void unbind() const;

private:
    GLuint                                                m_ProgramId;
    std::vector<std::unique_ptr<OpenGLShaderProgramSlot>> m_DataSlots;
    std::vector<OpenGLShaderProgramTextureSlot>           m_TextureSlots;
};

}  // namespace Engine