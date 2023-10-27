#pragma once

#include "OpenGLDepthStencilTexture.hpp"
#include "OpenGLRenderTexture.hpp"
#include "OpenGLFramebuffer.hpp"
#include "OpenGLShaderProgram.hpp"
#include "OpenGLRenderPass.hpp"

#include <vector>
#include <memory>
#include <unordered_map>

namespace Engine {

class OpenGLRender {
  private:
    HWND m_Window;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    D3D12_VIEWPORT m_ScreenViewport; 
    D3D12_RECT m_ScissorRect;

    std::shared_ptr<OpenGLRender> m_RenderPass;
    std::shared_ptr<OpenGLFramebuffer> m_Framebuffer;

    DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    bool m_4xMsaaState = false;    // 4X MSAA enabled
    uint32_t m_4xMsaaQuality = 0;  // quality level of 4X MSAA

    std::unique_ptr<GeometryRegistry> m_GeometryRegistry;
    std::unique_ptr<CBPool> m_CBPool;

  public:
    OpenGLRender(void* window, uint32_t width, uint32_t height);

    virtual ~OpenGLRender();

    void beginFrame();

    void endFrame();

    void flushCommandQueue();

    void resize(uint32_t width, uint32_t height);

    void clear(float r, float g, float b, float a);

    void setRenderPass(std::shared_ptr<OpenGLRenderPass> pass);

    void setFramebuffer(std::shared_ptr<penGLFramebuffer> fb);

    void registerGeometry(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes);

    void drawItem(const std::string& geometry, const std::string& subGeometry);

    void setShaderParameterTexture(size_t index, D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor);

    template<typename T>
    void setShaderParameterData(size_t index, T data) {
      auto cbRecord = m_CBPool->get<T>();
      cbRecord->buffer->CopyData(0, data);
			m_CommandList->SetGraphicsRootConstantBufferView(index, cbRecord->buffer->resource()->GetGPUVirtualAddress());
      m_CBPool->release(cbRecord);      
    }
};

} // namespace Engine
