#include "DxRenderTexture.hpp"

#include <stdexcept>

namespace Engine {

DxRenderTexture::DxRenderTexture(
    Microsoft::WRL::ComPtr<ID3D12Resource> resource,
    DXGI_FORMAT format,
    D3D12_RESOURCE_FLAGS flags,
    size_t width,
    size_t height,
    ID3D12Device* device,
    DxDescriptorPool* srvDescPool,
    DxDescriptorPool* rtvDescPool
) {
    m_Resource = resource;
    m_Device = device;
    m_SrvDescPool = srvDescPool;
    m_RtvDescPool = rtvDescPool;

    m_State = D3D12_RESOURCE_STATE_COMMON;
    m_SrvDescriptor = srvDescPool->get();
    m_RtvDescriptor = rtvDescPool->get();
    m_Format = format;
    m_Flags = flags;

    m_Device->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RtvDescriptor.cpu);
    m_Device->CreateShaderResourceView(m_Resource.Get(), nullptr, m_SrvDescriptor.cpu);

    m_Width = width;
    m_Height = height;
}

DxRenderTexture::DxRenderTexture(
    DXGI_FORMAT format,
    D3D12_RESOURCE_FLAGS flags,
    size_t width,
    size_t height,
    ID3D12Device* device,
    DxDescriptorPool* srvDescPool,
    DxDescriptorPool* rtvDescPool
) {
    m_Device = device;
    m_SrvDescPool = srvDescPool;
    m_RtvDescPool = rtvDescPool;

    m_State = D3D12_RESOURCE_STATE_COMMON;
    m_SrvDescriptor = srvDescPool->get();
    m_RtvDescriptor = rtvDescPool->get();
    m_Format = format;
    m_Flags = flags;

    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { m_Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
    if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))) {
        throw std::runtime_error("CheckFeatureSupport");
    }

    UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
    if ((formatSupport.Support1 & required) != required) {
#ifdef _DEBUG
        char buff[128] = {};
        sprintf_s(buff, "DxRenderTexture: Device does not support the requested format (%u)!\n", m_Format);
        OutputDebugStringA(buff);
#endif
        throw std::runtime_error("DxRenderTexture");
    }

    setClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    resize(width, height);
}

void DxRenderTexture::resize(size_t width, size_t height) {
    if (width == m_Width && height == m_Height) {
        return;
    }

    if (width > UINT32_MAX || height > UINT32_MAX) {
        throw std::out_of_range("Invalid width/height");
    }

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(m_Format,
        static_cast<UINT64>(width),
        static_cast<UINT>(height),
        1, 1, 1, 0, m_Flags);

    D3D12_CLEAR_VALUE clearValue = { m_Format, {} };
    memcpy(clearValue.Color, m_ClearColor, sizeof(clearValue.Color));

    m_State = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Create a render target
    ThrowIfFailed(
        m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            m_State, &clearValue,
            IID_PPV_ARGS(m_Resource.ReleaseAndGetAddressOf())
        )
    );

    // SetDebugObjectName(m_resource.Get(), L"DxRenderTexture RT");

    // Create RTV.
    m_Device->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RtvDescriptor.cpu);

    // Create SRV.
    m_Device->CreateShaderResourceView(m_Resource.Get(), nullptr, m_SrvDescriptor.cpu);

    m_Width = width;
    m_Height = height;
}

void DxRenderTexture::release() {
    m_Resource.Reset();

    m_SrvDescPool->release(m_SrvDescriptor);
    m_RtvDescPool->release(m_RtvDescriptor);

    m_State = D3D12_RESOURCE_STATE_COMMON;
    m_Width = m_Height = 0;

    m_SrvDescriptor.cpu.ptr = m_RtvDescriptor.cpu.ptr = 0;
    m_SrvDescriptor.gpu.ptr = m_RtvDescriptor.gpu.ptr = 0;
}

void DxRenderTexture::transitionTo(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState) {
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), m_State, afterState));
    m_State = afterState;
}

void DxRenderTexture::clear(ID3D12GraphicsCommandList* commandList) {
    commandList->ClearRenderTargetView(m_RtvDescriptor.cpu, m_ClearColor, 0, nullptr);
}

} // namespace Engine