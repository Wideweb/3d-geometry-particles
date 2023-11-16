#pragma once

#include "DxDescriptorPool.hpp"
#include "DxResource.hpp"
#include "DxUtils.hpp"

namespace Engine {

class DxRenderTexture : public DxResource {
public:
    DxRenderTexture(
        Microsoft::WRL::ComPtr<ID3D12Resource> resource, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, size_t width,
        size_t height, ID3D12Device* device, DxDescriptorPool* srvDescPool, DxDescriptorPool* rtvDescPool,
        DxDescriptorPool* uavDescPool
    );

    DxRenderTexture(
        DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, size_t width, size_t height, ID3D12Device* device,
        DxDescriptorPool* srvDescPool, DxDescriptorPool* rtvDescPool, DxDescriptorPool* uavDescPool
    );

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

    ID3D12Resource*       getResource() const noexcept { return m_Resource.Get(); }
    D3D12_RESOURCE_STATES getCurrentState() const noexcept { return m_State; }

    DXGI_FORMAT          getFormat() const noexcept { return m_Format; }
    D3D12_RESOURCE_FLAGS getFlags() const noexcept { return m_Flags; }

    DxDescriptor getSrvDescriptor() const noexcept { return m_SrvDescriptor; }
    DxDescriptor getRtvDescriptor() const noexcept { return m_RtvDescriptor; }
    DxDescriptor getUavDescriptor() const noexcept { return m_UavDescriptor; }

    const float* getClearColor() const noexcept { return m_ClearColor; }

private:
    ID3D12Device*     m_Device;
    DxDescriptorPool* m_SrvDescPool;
    DxDescriptorPool* m_RtvDescPool;
    DxDescriptorPool* m_UavDescPool;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
    D3D12_RESOURCE_STATES                  m_State;
    DxDescriptor                           m_SrvDescriptor;
    DxDescriptor                           m_RtvDescriptor;
    DxDescriptor                           m_UavDescriptor;
    float                                  m_ClearColor[4];

    DXGI_FORMAT          m_Format;
    D3D12_RESOURCE_FLAGS m_Flags;

    size_t m_Width;
    size_t m_Height;
};

}  // namespace Engine