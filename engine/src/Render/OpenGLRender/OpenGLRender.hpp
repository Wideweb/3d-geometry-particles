#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "OpenGLDepthStencilTexture.hpp"
#include "OpenGLFramebuffer.hpp"
#include "OpenGLRenderPass.hpp"
#include "OpenGLRenderTexture.hpp"
#include "OpenGLShaderProgram.hpp"

namespace Engine {

class OpenGLRender {
private:
    void* m_Window;

    uint32_t m_Width  = 0;
    uint32_t m_Height = 0;

    std::shared_ptr<OpenGLRender>      m_RenderPass;
    std::shared_ptr<OpenGLFramebuffer> m_Framebuffer;

    bool     m_4xMsaaState   = false;  // 4X MSAA enabled
    uint32_t m_4xMsaaQuality = 0;      // quality level of 4X MSAA

    std::unique_ptr<OpenGLGeometryRegistry> m_GeometryRegistry;

public:
    OpenGLRender(void* window, uint32_t width, uint32_t height);

    virtual ~OpenGLRender();

    void beginInitialization();

    void endInitialization();

    void beginFrame();

    void endFrame();

    void resize(uint32_t width, uint32_t height);

    void clear(float r, float g, float b, float a);

    void setPass(std::shared_ptr<OpenGLRenderPass> pass);

    void setFramebuffer(std::shared_ptr<OpenGLFramebuffer> fb);

    void registerGeometry(
        const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
    );

    void drawItem(const std::string& geometry, const std::string& subGeometry);
};

}  // namespace Engine
