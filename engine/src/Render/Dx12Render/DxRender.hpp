#pragma once

#include "DxComputePass.hpp"
#include "DxComputeProgram.hpp"
#include "DxDepthStencilTexture.hpp"
#include "DxDescriptorPool.hpp"
#include "DxFramebuffer.hpp"
#include "DxGeometryRegistry.hpp"
#include "DxReadWriteDataBuffer.hpp"
#include "DxRenderPass.hpp"
#include "DxRenderResource.hpp"
#include "DxRenderTexture.hpp"
#include "DxShaderProgram.hpp"
#include "DxShaderProgramDataBuffer.hpp"
#include "DxTexture.hpp"

#define NOMINMAX
#include <windows.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace Engine {

class DxRender {
private:
    HWND m_Window;

    uint32_t m_Width  = 0;
    uint32_t m_Height = 0;

    D3D12_VIEWPORT m_ScreenViewport;
    D3D12_RECT     m_ScissorRect;

    Microsoft::WRL::ComPtr<IDXGIFactory4>  m_DxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device>   m_Device;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    uint64_t                            m_CurrentFence = 0;

    uint64_t m_CurrFrameIndex = 0;

    std::unique_ptr<DxRenderResource> m_RenderResource;
    std::shared_ptr<DxRenderPass>     m_RenderPass;
    std::shared_ptr<DxFramebuffer>    m_Framebuffer;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue>        m_CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_DirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

    static const int c_SwapChainBufferCount = 2;
    uint32_t         m_CurrBackBuffer       = 0;

    std::unique_ptr<DxRenderTexture>       m_SwapChainBuffers[c_SwapChainBufferCount];
    std::unique_ptr<DxDepthStencilTexture> m_DepthStencilBuffer;

    uint32_t m_RtvDescriptorSize = 0;

    std::unique_ptr<DxDescriptorPool> m_RtvDescPool;
    std::unique_ptr<DxDescriptorPool> m_DsvDescPool;
    std::unique_ptr<DxDescriptorPool> m_CbvSrvUavDescPool;
    std::unique_ptr<DxDescriptorPool> m_CbvDescPool;
    std::unique_ptr<DxDescriptorPool> m_SrvDescPool;
    std::unique_ptr<DxDescriptorPool> m_UavDescPool;

    DXGI_FORMAT m_BackBufferFormat   = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    bool     m_4xMsaaState   = false;  // 4X MSAA enabled
    uint32_t m_4xMsaaQuality = 0;      // quality level of 4X MSAA

    std::unique_ptr<DxGeometryRegistry> m_GeometryRegistry;

    ID3D12Resource*             currentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView() const;

public:
    DxRender(void* window, uint32_t width, uint32_t height);

    virtual ~DxRender();

    void beginInitialization();

    void endInitialization();

    void beginFrame();

    void endFrame();

    void flushCommandQueue();

    void resize(uint32_t width, uint32_t height);

    void clear(float r, float g, float b, float a);

    void setViewport(uint32_t width, uint32_t height);

    void setRenderPass(std::shared_ptr<DxRenderPass> pass);

    void setComputePass(std::shared_ptr<DxComputePass> pass);

    void setFramebuffer(std::shared_ptr<DxFramebuffer> fb);

    void registerGeometry(
        const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
    );

    std::shared_ptr<DxTexture> loadTexture(const std::string& filename);

    std::shared_ptr<DxTexture> loadCubeTexture(const std::array<std::string, 6>& files);

    std::shared_ptr<DxDepthStencilTexture> createDepthStencilTexture(DXGI_FORMAT format, size_t width, size_t height);

    std::shared_ptr<DxRenderTexture> createRenderTexture(
        DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, size_t width, size_t height
    );

    std::shared_ptr<DxShaderProgram> createShaderProgram(
        const std::string& vertexFile, const std::string& pixelFile, const std::vector<ShaderProgramSlotDesc>& slots
    );

    std::shared_ptr<DxComputeProgram> createComputeProgram(
        const std::string& file, const std::vector<ShaderProgramSlotDesc>& slots
    );

    std::shared_ptr<DxShaderProgramDataBuffer> createShaderProgramDataBuffer(size_t byteSize);

    std::shared_ptr<DxReadWriteDataBuffer> createReadWriteDataBuffer(size_t byteSize);

    std::shared_ptr<DxRenderPass> createRenderPass(
        std::shared_ptr<DxShaderProgram> shaderProgram, DxRenderPass::PipelineDesc desc
    );

    std::shared_ptr<DxRenderPass> createRenderPass(
        std::shared_ptr<DxShaderProgram> shaderProgram, std::vector<DXGI_FORMAT> rtvFormats, DXGI_FORMAT dsvFormat,
        DxRenderPass::PipelineDesc desc
    );

    std::shared_ptr<DxComputePass> createComputePass(std::shared_ptr<DxComputeProgram> computeProgram);

    void drawItem(const std::string& geometry, const std::string& subGeometry);

    void getViewport(uint32_t& width, uint32_t& height) {
        width  = m_Width;
        height = m_Height;
    }

    void compute(size_t x, size_t y, size_t z);
};

}  // namespace Engine
