#include "DepthStencilTexture.hpp"

#include <stdexcept>

DepthStencilTexture::DepthStencilTexture(
    DXGI_FORMAT format,
    size_t width,
    size_t height,
    ID3D12Device* device,
    DescriptorPool* srvDescPool,
    DescriptorPool* dsvDescPool
) {
    m_Device = device;
    m_SrvDescPool = srvDescPool;
    m_DsvDescPool = dsvDescPool;

    m_State = D3D12_RESOURCE_STATE_COMMON;
    m_SrvDescriptor = srvDescPool->get();
    m_DsvDescriptor = dsvDescPool->get();
    m_Format = format;
    m_ClearDepthValue = 1.0f;
    m_ClearStencilValue = 0;

    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { m_Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
    if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))) {
        throw std::runtime_error("CheckFeatureSupport");
    }

    UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
    if ((formatSupport.Support1 & required) != required) {
#ifdef _DEBUG
        char buff[128] = {};
        sprintf_s(buff, "DepthStencilTexture: Device does not support the requested format (%u)!\n", m_Format);
        OutputDebugStringA(buff);
#endif
        throw std::runtime_error("DepthStencilTexture");
    }

    resize(width, height);
}

void DepthStencilTexture::resize(size_t width, size_t height) {
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
        1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = m_Format;
    clearValue.DepthStencil.Depth = m_ClearDepthValue;
    clearValue.DepthStencil.Stencil = m_ClearStencilValue;

    m_State = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Create a render target
    ThrowIfFailed(
        m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            m_State, &clearValue,
            IID_PPV_ARGS(m_Resource.ReleaseAndGetAddressOf())
        )
    );

    // SetDebugObjectName(m_resource.Get(), L"DepthStencilTexture RT");

    // Create RTV.
    m_Device->CreateRenderTargetView(m_Resource.Get(), nullptr, m_DsvDescriptor);

    // Create SRV.
    m_Device->CreateShaderResourceView(m_Resource.Get(), nullptr, m_SrvDescriptor);

    m_Width = width;
    m_Height = height;
}

void DepthStencilTexture::release() {
    m_Resource.Reset();

    m_SrvDescPool->release(m_SrvDescriptor);
    m_DsvDescPool->release(m_DsvDescriptor);

    m_State = D3D12_RESOURCE_STATE_COMMON;
    m_Width = m_Height = 0;

    m_SrvDescriptor.ptr = m_DsvDescriptor.ptr = 0;
}

void DepthStencilTexture::transitionTo(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState) {
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), m_State, afterState));
    m_State = afterState;
}

void DepthStencilTexture::clear(ID3D12GraphicsCommandList* commandList) {
    commandList->ClearDepthStencilView(
        m_DsvDescriptor,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        m_ClearDepthValue,
        m_ClearStencilValue,
        0,
        nullptr
    );
}