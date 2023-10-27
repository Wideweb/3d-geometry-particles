#pragma once

#include "DxUtils.hpp"
#include "DescriptorPool.hpp"

class DepthStencilTexture {
public:
    DepthStencilTexture(
        DXGI_FORMAT format,
        size_t width,
        size_t height,
        ID3D12Device* device,
        DescriptorPool* srvDescPool,
        DescriptorPool* dsvDescPool);

    void resize(size_t width, size_t height);

    void release();

    void transitionTo(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState);

    void beginRenderTo(ID3D12GraphicsCommandList* commandList) {
        transitionTo(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    void endRenderTo(ID3D12GraphicsCommandList* commandList) {
        transitionTo(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

        void setClearValue(float depth, uint8_t stencil) {
        m_ClearDepthValue = depth;
        m_ClearStencilValue = stencil;
    }

    void clear(ID3D12GraphicsCommandList* commandList);

    ID3D12Resource* getResource() const noexcept { return m_Resource.Get(); }
    D3D12_RESOURCE_STATES getCurrentState() const noexcept { return m_State; }

    DXGI_FORMAT getFormat() const noexcept { return m_Format; }

    D3D12_CPU_DESCRIPTOR_HANDLE getSrvDescriptor() const noexcept { return m_SrvDescriptor; }
    D3D12_CPU_DESCRIPTOR_HANDLE getDsvDescriptor() const noexcept { return m_DsvDescriptor; }

    float getClearDepthValue() const noexcept { return m_ClearDepthValue; }
    float getClearStencilValue() const noexcept { return m_ClearStencilValue; }

private:
    ID3D12Device*                                       m_Device;
    DescriptorPool*                                     m_SrvDescPool;
    DescriptorPool*                                     m_DsvDescPool;

    Microsoft::WRL::ComPtr<ID3D12Resource>              m_Resource;
    D3D12_RESOURCE_STATES                               m_State;
    CD3DX12_CPU_DESCRIPTOR_HANDLE                       m_SrvDescriptor;
    CD3DX12_CPU_DESCRIPTOR_HANDLE                       m_DsvDescriptor;
    float                                               m_ClearDepthValue;
    uint8_t                                             m_ClearStencilValue;

    DXGI_FORMAT                                         m_Format;

    size_t                                              m_Width;
    size_t                                              m_Height;
};