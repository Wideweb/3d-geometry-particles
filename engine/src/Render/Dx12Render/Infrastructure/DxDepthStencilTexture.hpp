#pragma once

#include "DxUtils.hpp"
#include "DescriptorPool.hpp"

namespace Engine {

class DxDepthStencilTexture {
public:
    DxDepthStencilTexture(
        DXGI_FORMAT format,
        size_t width,
        size_t height,
        ID3D12Device* device,
        DxDescriptorPool* srvDescPool,
        DxDescriptorPool* dsvDescPool);

    void resize(size_t width, size_t height);

    void release();

    void transitionTo(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState);

    void beginRenderTo(ID3D12GraphicsCommandList* commandList) {
        transitionTo(commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    void endRenderTo(ID3D12GraphicsCommandList* commandList) {
        // D3D12_RESOURCE_STATE_COMMON
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

    DxDescriptor getSrvDescriptor() const noexcept { return m_SrvDescriptor; }
    DxDescriptor getDsvDescriptor() const noexcept { return m_DsvDescriptor; }

    float getClearDepthValue() const noexcept { return m_ClearDepthValue; }
    float getClearStencilValue() const noexcept { return m_ClearStencilValue; }

private:
    ID3D12Device*                                       m_Device;
    DescriptorPool*                                     m_SrvDescPool;
    DescriptorPool*                                     m_DsvDescPool;

    Microsoft::WRL::ComPtr<ID3D12Resource>              m_Resource;
    D3D12_RESOURCE_STATES                               m_State;
    DxDescriptor                                        m_SrvDescriptor;
    DxDescriptor                                        m_DsvDescriptor;
    float                                               m_ClearDepthValue;
    uint8_t                                             m_ClearStencilValue;

    DXGI_FORMAT                                         m_Format;

    size_t                                              m_Width;
    size_t                                              m_Height;
};

} // namespace Engine