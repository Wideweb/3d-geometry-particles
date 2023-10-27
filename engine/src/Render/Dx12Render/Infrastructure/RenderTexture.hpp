#pragma once

#include "DxUtils.hpp"
#include "DescriptorPool.hpp"

class RenderTexture {
public:
    RenderTexture(
        DXGI_FORMAT format,
        D3D12_RESOURCE_FLAGS flags,
        size_t width,
        size_t height,
        ID3D12Device* device,
        DescriptorPool* srvDescPool,
        DescriptorPool* rtvDescPool);

    void resize(size_t width, size_t height);

    void release();

    void transitionTo(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState);

    void beginRenderTo(ID3D12GraphicsCommandList* commandList) {
        transitionTo(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    void endRenderTo(ID3D12GraphicsCommandList* commandList) {
        transitionTo(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    void setClearColor(float r, float g, float b, float a) {
        m_ClearColor[0] = r;
        m_ClearColor[1] = g;
        m_ClearColor[2] = b;
        m_ClearColor[3] = a;
    }

    void clear(ID3D12GraphicsCommandList* commandList);

    ID3D12Resource* getResource() const noexcept { return m_Resource.Get(); }
    D3D12_RESOURCE_STATES getCurrentState() const noexcept { return m_State; }

    DXGI_FORMAT getFormat() const noexcept { return m_Format; }
    D3D12_RESOURCE_FLAGS getFlags() const noexcept { return m_Flags; }

    CD3DX12_CPU_DESCRIPTOR_HANDLE getSrvDescriptor() const noexcept { return m_SrvDescriptor; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE getRtvDescriptor() const noexcept { return m_RtvDescriptor; }

    const float* getClearColor() const noexcept { return m_ClearColor; }

private:
    ID3D12Device*                                       m_Device;
    DescriptorPool*                                     m_SrvDescPool;
    DescriptorPool*                                     m_RtvDescPool;

    Microsoft::WRL::ComPtr<ID3D12Resource>              m_Resource;
    D3D12_RESOURCE_STATES                               m_State;
    CD3DX12_CPU_DESCRIPTOR_HANDLE                       m_SrvDescriptor;
    CD3DX12_CPU_DESCRIPTOR_HANDLE                       m_RtvDescriptor;
    float                                               m_ClearColor[4];

    DXGI_FORMAT                                         m_Format;
    D3D12_RESOURCE_FLAGS                                m_Flags;

    size_t                                              m_Width;
    size_t                                              m_Height;
};