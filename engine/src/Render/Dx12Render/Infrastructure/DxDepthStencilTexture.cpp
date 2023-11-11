#include "DxDepthStencilTexture.hpp"

#include <stdexcept>

namespace Engine {

DxDepthStencilTexture::DxDepthStencilTexture(
    DXGI_FORMAT format, size_t width, size_t height, ID3D12Device* device, DxDescriptorPool* srvDescPool,
    DxDescriptorPool* dsvDescPool
) {
    m_Device      = device;
    m_SrvDescPool = srvDescPool;
    m_DsvDescPool = dsvDescPool;

    m_State             = D3D12_RESOURCE_STATE_COMMON;
    m_SrvDescriptor     = srvDescPool->get();
    m_DsvDescriptor     = dsvDescPool->get();
    m_Format            = format;
    m_ClearDepthValue   = 1.0f;
    m_ClearStencilValue = 0;

    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {
        m_Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
    };
    if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))) {
        throw std::runtime_error("CheckFeatureSupport");
    }

    UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL;
    if ((formatSupport.Support1 & required) != required) {
#ifdef _DEBUG
        char buff[128] = {};
        sprintf_s(buff, "DxDepthStencilTexture: Device does not support the requested format (%u)!\n", m_Format);
        OutputDebugStringA(buff);
#endif
        throw std::runtime_error("DxDepthStencilTexture");
    }

    resize(width, height);
}

void DxDepthStencilTexture::resize(size_t width, size_t height) {
    if (width == 0 || height == 0) {
        return;
    }

    if (width == m_Width && height == m_Height) {
        return;
    }

    if (width > UINT32_MAX || height > UINT32_MAX) {
        throw std::out_of_range("Invalid width/height");
    }

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    // D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(m_Format,
    //   static_cast<UINT64>(width),
    // static_cast<UINT>(height),
    // 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_RESOURCE_DESC desc;
    desc.Format             = m_Format;
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment          = 0;
    desc.Width              = static_cast<UINT64>(width);
    desc.Height             = static_cast<UINT>(height);
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format               = m_Format;
    clearValue.DepthStencil.Depth   = m_ClearDepthValue;
    clearValue.DepthStencil.Stencil = m_ClearStencilValue;

    m_State = D3D12_RESOURCE_STATE_COMMON;

    // Create a render target
    ThrowIfFailed(m_Device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &desc, m_State, &clearValue,
        IID_PPV_ARGS(m_Resource.ReleaseAndGetAddressOf())
    ));

    // SetDebugObjectName(m_resource.Get(), L"DxDepthStencilTexture RT");

    // Create DSV.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format             = m_Format;
    dsvDesc.Texture2D.MipSlice = 0;

    m_Device->CreateDepthStencilView(m_Resource.Get(), &dsvDesc, m_DsvDescriptor.cpu);

    // Create SRV.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format                          = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip       = 0;
    srvDesc.Texture2D.MipLevels             = 1;
    srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;
    srvDesc.Texture2D.PlaneSlice            = 0;

    m_Device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, m_SrvDescriptor.cpu);

    m_Width  = width;
    m_Height = height;
}

void DxDepthStencilTexture::release() {
    m_Resource.Reset();

    m_SrvDescPool->release(m_SrvDescriptor);
    m_DsvDescPool->release(m_DsvDescriptor);

    m_State = D3D12_RESOURCE_STATE_COMMON;
    m_Width = m_Height = 0;

    m_SrvDescriptor.cpu.ptr = m_DsvDescriptor.cpu.ptr = 0;
    m_SrvDescriptor.gpu.ptr = m_DsvDescriptor.gpu.ptr = 0;
}

void DxDepthStencilTexture::transitionTo(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState) {
    if (m_State == afterState) {
        return;
    }

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), m_State, afterState);
    commandList->ResourceBarrier(1, &barrier);
    m_State = afterState;
}

void DxDepthStencilTexture::clear(ID3D12GraphicsCommandList* commandList) {
    commandList->ClearDepthStencilView(
        m_DsvDescriptor.cpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, m_ClearDepthValue, m_ClearStencilValue,
        0, nullptr
    );
}

}  // namespace Engine