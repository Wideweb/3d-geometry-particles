#include "DxRender.hpp"

#include <WindowsX.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <string_view>

#include "DxTextureLoader.hpp"
#include "DxUtils.hpp"

namespace Engine {

DxRender::DxRender(void* window, uint32_t width, uint32_t height)
: m_Window((HWND)window),
  m_Width(width),
  m_Height(height) {
    using namespace std;
    using namespace std::string_view_literals;
    using Microsoft::WRL::ComPtr;

// Enable the D3D12 debug layer.
#if defined(DEBUG) || defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DxgiFactory)));

    // Try to create hardware device.
    HRESULT hardwareResult = D3D12CreateDevice(
        nullptr,                 // NULL to use the default adapter
        D3D_FEATURE_LEVEL_11_0,  // The minimum D3D_FEATURE_LEVEL
        IID_PPV_ARGS(&m_Device)  //
    );

    // Fallback to WARP device.
    if (hardwareResult < 0) {
        ComPtr<IDXGIAdapter> pWarpAdapter;
        ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)));
    }

    ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

    // Check 4X MSAA quality support for our back buffer format.
    // All Direct3D 11 capable devices support 4X MSAA for all render
    // target formats, so we only need to check quality support.

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format           = m_BackBufferFormat;
    msQualityLevels.SampleCount      = 4;
    msQualityLevels.Flags            = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(m_Device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)
    ));

    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

    //////////////////////// COMANDS ///////////////////////

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    // Specifies a command buffer that the GPU can execute. A direct command list doesn't inherit any GPU state.
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    // Default command queue.
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

    ThrowIfFailed(m_Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())
    ));

    ThrowIfFailed(m_Device->CreateCommandList(
        0,                               // For single-GPU operation, set this to zero.
        D3D12_COMMAND_LIST_TYPE_DIRECT,  // An optional pointer to the pipeline state object that contains the initial
                                         // pipeline state for the command list.
        m_DirectCmdListAlloc.Get(),      // Associated command allocator
        nullptr,                         // Initial PipelineStateObject
        IID_PPV_ARGS(m_CommandList.GetAddressOf())
    ));

    // Start off in a closed state.  This is because the first time we refer
    // to the command list we will Reset it, and it needs to be closed before
    // calling Reset.
    m_CommandList->Close();

    //////////////////////// SWAP CHAIN ///////////////////////
    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width                   = m_Width;
    sd.BufferDesc.Height                  = m_Height;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format                  = m_BackBufferFormat;
    sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
    // sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    // sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount        = c_SwapChainBufferCount;
    sd.OutputWindow       = m_Window;
    sd.Windowed           = true;
    sd.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;  // Discard the contents of the back buffer after.
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // When switching from windowed to full-screen mode, the display
                                                        // mode (or monitor resolution) will be changed to match the
                                                        // dimensions of the application window.

    // Note: Swap chain uses queue to perform flush.
    ThrowIfFailed(m_DxgiFactory->CreateSwapChain(m_CommandQueue.Get(), &sd, m_SwapChain.GetAddressOf()));

    //////////////////////// DESCRIPTOR HEAPS ///////////////////////
    m_RtvDescPool = std::make_unique<DxDescriptorPool>(
        m_Device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 1000
    );

    m_DsvDescPool = std::make_unique<DxDescriptorPool>(
        m_Device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 1000
    );

    m_CbvSrvUavDescPool = std::make_unique<DxDescriptorPool>(
        m_Device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 900
    );

    m_CbvDescPool = std::make_unique<DxDescriptorPool>(m_CbvSrvUavDescPool.get(), 0, 299);
    m_SrvDescPool = std::make_unique<DxDescriptorPool>(m_CbvSrvUavDescPool.get(), 301, 600);
    m_UavDescPool = std::make_unique<DxDescriptorPool>(m_CbvSrvUavDescPool.get(), 601, 899);

    m_GeometryRegistry = std::make_unique<DxGeometryRegistry>(m_Device.Get());
    m_RenderResource   = std::make_unique<DxRenderResource>(m_Device.Get());

    resize(m_Width, m_Height);
}

void DxRender::resize(uint32_t width, uint32_t height) {
    assert(m_Device);
    assert(m_SwapChain);
    assert(m_DirectCmdListAlloc);

    m_Width  = width;
    m_Height = height;

    // Flush before changing any resources.
    flushCommandQueue();

    for (size_t i = 0; i < c_SwapChainBufferCount; i++) {
        if (m_SwapChainBuffers[i] != nullptr) {
            m_SwapChainBuffers[i]->release();
        }
    }

    if (m_DepthStencilBuffer != nullptr) {
        m_DepthStencilBuffer->release();
    }

    ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

    // Resize the swap chain.
    ThrowIfFailed(m_SwapChain->ResizeBuffers(
        c_SwapChainBufferCount, m_Width, m_Height, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
    ));

    m_CurrBackBuffer = 0;

    for (size_t i = 0; i < c_SwapChainBufferCount; i++) {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource)));

        m_SwapChainBuffers[i] = std::make_unique<DxRenderTexture>(
            resource, m_BackBufferFormat, D3D12_RESOURCE_FLAG_NONE, m_Width, m_Height, m_Device.Get(),
            m_SrvDescPool.get(), m_RtvDescPool.get(), m_UavDescPool.get()
        );
    }

    m_DepthStencilBuffer = std::make_unique<DxDepthStencilTexture>(
        m_DepthStencilFormat, m_Width, m_Height, m_Device.Get(), m_SrvDescPool.get(), m_DsvDescPool.get()
    );
    m_DepthStencilBuffer->beginRenderTo(m_CommandList.Get());

    setViewport(m_Width, m_Height);

    // Execute the resize commands.
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = {m_CommandList.Get()};
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until resize is complete.
    flushCommandQueue();
}

DxRender::~DxRender() {
    if (m_Device != nullptr) {
        flushCommandQueue();
    }
}

void DxRender::beginInitialization() { ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr)); }

void DxRender::endInitialization() {
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = {m_CommandList.Get()};
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    flushCommandQueue();
}

void DxRender::beginFrame() {
    m_RenderResource->beginFrame(m_CurrFrameIndex, m_Fence.Get());

    auto& cmdListAlloc = m_RenderResource->currFrameResource->cmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_RenderPass != nullptr ? m_RenderPass->resource() : nullptr)
    );

    m_CommandList->RSSetViewports(1, &m_ScreenViewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    // Indicate a state transition on the resource usage.
    auto toRTBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        currentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_CommandList->ResourceBarrier(1, &toRTBarrier);

    auto rtDescriptor = currentBackBufferView();
    auto dsDescriptor = m_DepthStencilBuffer->getDsvDescriptor().cpu;
    m_CommandList->OMSetRenderTargets(1, &rtDescriptor, true, &dsDescriptor);

    ID3D12DescriptorHeap* descriptorHeaps[] = {m_CbvSrvUavDescPool->getHeap()};
    m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);
}

void DxRender::endFrame() {
    // Indicate a state transition on the resource usage.
    auto toPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        currentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
    );
    m_CommandList->ResourceBarrier(1, &toPresentBarrier);

    // Done recording commands.
    ThrowIfFailed(m_CommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = {m_CommandList.Get()};
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(m_SwapChain->Present(0, 0));
    m_CurrBackBuffer = (m_CurrBackBuffer + 1) % c_SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    m_RenderResource->setFence(++m_CurrentFence);

    // Add an instruction to the command queue to set a new fence point.
    // Because we are on the GPU timeline, the new fence point won't be
    // set until the GPU finishes processing all the commands prior to this Signal().
    m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);

    // Cycle through the circular frame resource array.
    m_CurrFrameIndex = (m_CurrFrameIndex + 1) % 3;

    this->flushCommandQueue();
}

void DxRender::flushCommandQueue() {
    // Advance the fence value to mark commands up to this fence point.
    m_CurrentFence++;

    // Add an instruction to the command queue to set a new fence point. Because we
    // are on the GPU timeline, the new fence point won't be set until the GPU finishes
    // processing all the commands prior to this Signal().
    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

    auto f = m_Fence->GetCompletedValue();

    // Wait until the GPU has completed commands up to this fence point.
    if (m_Fence->GetCompletedValue() < m_CurrentFence) {
        HANDLE eventHandle = CreateEventEx(nullptr, "FLUSH", false, EVENT_ALL_ACCESS);

        // Fire event when GPU hits current fence.
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

        // Wait until the GPU hits current fence event is fired.
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

std::shared_ptr<DxRenderPass> DxRender::createRenderPass(
    std::shared_ptr<DxShaderProgram> shaderProgram, DxRenderPass::PipelineDesc desc
) {
    std::vector<DXGI_FORMAT> rtvFormats = {m_BackBufferFormat};
    auto pass = std::make_shared<DxRenderPass>(m_Device.Get(), shaderProgram, rtvFormats, m_DepthStencilFormat, desc);
    return pass;
}

std::shared_ptr<DxRenderPass> DxRender::createRenderPass(
    std::shared_ptr<DxShaderProgram> shaderProgram, std::vector<DXGI_FORMAT> rtvFormats, DXGI_FORMAT dsvFormat,
    DxRenderPass::PipelineDesc desc
) {
    auto pass = std::make_shared<DxRenderPass>(m_Device.Get(), shaderProgram, rtvFormats, dsvFormat, desc);
    return pass;
}

std::shared_ptr<DxComputePass> DxRender::createComputePass(std::shared_ptr<DxComputeProgram> computeProgram) {
    auto pass = std::make_shared<DxComputePass>(m_Device.Get(), computeProgram);
    return pass;
}

std::shared_ptr<DxShaderProgram> DxRender::createShaderProgram(
    const std::string& vertexFile, const std::string& pixelFile, const std::vector<ShaderProgramSlotDesc>& slots
) {
    auto shader = std::make_shared<DxShaderProgram>(m_Device.Get(), vertexFile, pixelFile, slots);
    return shader;
}

std::shared_ptr<DxComputeProgram> DxRender::createComputeProgram(
    const std::string& file, const std::vector<ShaderProgramSlotDesc>& slots
) {
    auto shader = std::make_shared<DxComputeProgram>(m_Device.Get(), file, slots);
    return shader;
}

std::shared_ptr<DxShaderProgramDataBuffer> DxRender::createShaderProgramDataBuffer(size_t byteSize) {
    auto buffer = std::make_shared<DxShaderProgramDataBuffer>(m_Device.Get(), byteSize);
    return buffer;
}

std::shared_ptr<DxReadWriteDataBuffer> DxRender::createReadWriteDataBuffer(size_t byteSize) {
    auto buffer = std::make_shared<DxReadWriteDataBuffer>(m_Device.Get(), m_CommandList.Get(), byteSize);
    return buffer;
}

std::shared_ptr<DxRenderTexture> DxRender::createRenderTexture(
    DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, size_t width, size_t height
) {
    auto rt = std::make_shared<DxRenderTexture>(
        format, flags, width, height, m_Device.Get(), m_SrvDescPool.get(), m_RtvDescPool.get(), m_UavDescPool.get()
    );
    return rt;
}

std::shared_ptr<DxDepthStencilTexture> DxRender::createDepthStencilTexture(
    DXGI_FORMAT format, size_t width, size_t height
) {
    auto dst = std::make_shared<DxDepthStencilTexture>(
        format, width, height, m_Device.Get(), m_SrvDescPool.get(), m_DsvDescPool.get()
    );
    return dst;
}

std::shared_ptr<DxTexture> DxRender::loadTexture(const std::string& filename) {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap = nullptr;
    DxTextureLoader::loadImageDataFromFile(m_Device.Get(), m_CommandList.Get(), filename, resource, uploadHeap);
    auto texture = std::make_shared<DxTexture>(
        resource, D3D12_SRV_DIMENSION_TEXTURE2D, uploadHeap, m_Device.Get(), m_SrvDescPool.get()
    );
    return texture;
}

std::shared_ptr<DxTexture> DxRender::loadCubeTexture(const std::array<std::string, 6>& files) {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap = nullptr;
    DxTextureLoader::loadCubeMapDataFromFile(m_Device.Get(), m_CommandList.Get(), files, resource, uploadHeap);
    auto texture = std::make_shared<DxTexture>(
        resource, D3D12_SRV_DIMENSION_TEXTURECUBE, uploadHeap, m_Device.Get(), m_SrvDescPool.get()
    );
    return texture;
}

void DxRender::setRenderPass(std::shared_ptr<DxRenderPass> pass) {
    pass->bind(m_CommandList.Get());
    m_RenderPass = pass;
}

void DxRender::setComputePass(std::shared_ptr<DxComputePass> pass) { pass->bind(m_CommandList.Get()); }

void DxRender::setFramebuffer(std::shared_ptr<DxFramebuffer> fb) {
    if (m_Framebuffer != nullptr) {
        m_Framebuffer->endRenderTo(m_CommandList.Get());
    }

    if (fb == nullptr) {
        if (m_Framebuffer != nullptr) {
            // Specify the buffers we are going to render to.
            auto rtDescriptor = currentBackBufferView();
            auto dsDescriptor = m_DepthStencilBuffer->getDsvDescriptor().cpu;
            m_CommandList->OMSetRenderTargets(1, &rtDescriptor, true, &dsDescriptor);
        }
    } else {
        fb->beginRenderTo(m_CommandList.Get());
    }

    m_Framebuffer = fb;
}

void DxRender::clear(float r, float g, float b, float a) {
    const float clearColor[] = {r, g, b, a};

    if (m_Framebuffer == nullptr) {
        m_CommandList->ClearRenderTargetView(currentBackBufferView(), clearColor, 0, nullptr);
        m_CommandList->ClearDepthStencilView(
            m_DepthStencilBuffer->getDsvDescriptor().cpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0,
            nullptr
        );
    } else {
        m_Framebuffer->clear(m_CommandList.Get());
    }
}

void DxRender::setViewport(uint32_t width, uint32_t height) {
    m_Width  = width;
    m_Height = height;

    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width    = static_cast<float>(m_Width);
    m_ScreenViewport.Height   = static_cast<float>(m_Height);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    m_ScissorRect = {0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height)};

    m_CommandList->RSSetViewports(1, &m_ScreenViewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
}

void DxRender::registerGeometry(
    const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
) {
    m_GeometryRegistry->add(geometry, subGeometries, subMeshes, m_CommandList.Get());
}

void DxRender::drawItem(const std::string& geometry, const std::string& subGeometry) {
    const DxMeshGeometry*    geo    = m_GeometryRegistry->get(geometry);
    const DxSubmeshGeometry& subGeo = geo->drawArgs.at(subGeometry);

    auto vertexBufferView = geo->vertexBufferView();
    auto indexBufferView  = geo->indexBufferView();

    m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    m_CommandList->IASetIndexBuffer(&indexBufferView);
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    uint32_t indexCountPerInstance = subGeo.indexCount;
    uint32_t instanceCount         = 1;
    uint32_t startIndexLocation    = subGeo.startIndexLocation;
    uint32_t baseVertexLocation    = subGeo.baseVertexLocation;
    uint32_t startInstanceLocation = 0;

    m_CommandList->DrawIndexedInstanced(
        indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation
    );
}

void DxRender::compute(size_t x, size_t y, size_t z) {
    m_CommandList->Dispatch(x, y, z);
}

ID3D12Resource* DxRender::currentBackBuffer() const { return m_SwapChainBuffers[m_CurrBackBuffer]->getResource(); }

D3D12_CPU_DESCRIPTOR_HANDLE DxRender::currentBackBufferView() const {
    return m_SwapChainBuffers[m_CurrBackBuffer]->getRtvDescriptor().cpu;
}

}  // namespace Engine
