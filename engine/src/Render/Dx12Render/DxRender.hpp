#pragma once

#include "RenderResource.hpp"
#include "GeometryRegistry.hpp"
#include "CBPool.hpp"
#include "DescriptorPool.hpp"

#include "Texture.hpp"
#include "DepthStencilTexture.hpp"
#include "RenderTexture.hpp"
#include "Framebuffer.hpp"
#include "ShaderProgram.hpp"
#include "RenderPass.hpp"

#define NOMINMAX
#include <windows.h>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Engine {

class DxRender {
  private:
    HWND m_Window;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    D3D12_VIEWPORT m_ScreenViewport; 
    D3D12_RECT m_ScissorRect;

    Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    uint64_t m_CurrentFence = 0;

    uint64_t m_CurrFrameIndex = 0;

    std::unique_ptr<RenderResource> m_RenderResource;
    std::shared_ptr<RenderPass> m_RenderPass;
    std::shared_ptr<Framebuffer> m_Framebuffer;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

    static const int c_SwapChainBufferCount = 2;
    uint32_t m_CurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[c_SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

    uint32_t m_RtvDescriptorSize = 0;
    uint32_t m_DsvDescriptorSize = 0;
    uint32_t m_CbvSrvUavDescriptorSize = 0;

    std::unique_ptr<DescriptorPool> m_RtvDescPool;
    std::unique_ptr<DescriptorPool> m_DsvDescPool;
    std::unique_ptr<DescriptorPool> m_CbvSrvUavDescPool;

    DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    bool m_4xMsaaState = false;    // 4X MSAA enabled
    uint32_t m_4xMsaaQuality = 0;  // quality level of 4X MSAA

    std::unique_ptr<GeometryRegistry> m_GeometryRegistry;
    std::unique_ptr<CBPool> m_CBPool;

  public:
    DxRender(void* window, uint32_t width, uint32_t height);

    virtual ~DxRender();

    void beginFrame();

    void endFrame();

    void flushCommandQueue();

    void resize(uint32_t width, uint32_t height);

    void clear(float r, float g, float b, float a);

    void setRenderPass(std::shared_ptr<RenderPass> pass);

    void setFramebuffer(std::shared_ptr<Framebuffer> fb);

    void registerGeometry(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes);

    std::shared_ptr<DepthStencilTexture> createDepthStencilTexture(DXGI_FORMAT format, size_t width, size_t height);

    std::shared_ptr<RenderTexture> createRenderTexture(DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, size_t width, size_t height);

    std::shared_ptr<RenderPass> createRenderPass(std::shared_ptr<ShaderProgram> shaderProgram, size_t constantBuffersNum, size_t texturesNum);

    std::shared_ptr<RenderPass> createRenderPass(std::shared_ptr<ShaderProgram> shaderProgram, size_t constantBuffersNum, size_t texturesNum, std::vector<DXGI_FORMAT> rtvFormats, DXGI_FORMAT dsvFormat0);

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
