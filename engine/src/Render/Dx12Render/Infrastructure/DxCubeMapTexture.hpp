#pragma once

#include <string>

#include "DxDescriptorPool.hpp"
#include "DxResource.hpp"
#include "DxUtils.hpp"

namespace Engine {

class DxCubeMapTexture : public DxResource {
public:
    DxTexture(
        Microsoft::WRL::ComPtr<ID3D12Resource> resource, Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap,
        ID3D12Device* device, DxDescriptorPool* srvDescPool
    ) {
        m_Resource      = resource;
        m_UploadHead    = uploadHeap;
        m_SrvDescPool   = srvDescPool;
        m_SrvDescriptor = srvDescPool->get();

        std::vector<D3D12_SUBRESOURCE_DATA> subresources;
        for (int i = 0; i < 6; i++) {
            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData                  = textureData[i];  // данные текстуры
            subresourceData.RowPitch               = rowPitch;        // ширина строки
            subresourceData.SlicePitch             = slicePitch;      // размер среза
            subresources.push_back(subresourceData);
        }

        UpdateSubresources(commandList.Get(), cubeMap.Get(), 0, 0, 6, subresources.data());

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format                          = resource->GetDesc().Format;
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip       = 0;
        srvDesc.Texture2D.MipLevels             = 1;
        device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, m_SrvDescriptor.cpu);
    }

    void release() {
        m_Resource.Reset();
        m_SrvDescPool->release(m_SrvDescriptor);
        m_SrvDescriptor.cpu.ptr = m_SrvDescriptor.gpu.ptr = 0;
    }

    ID3D12Resource* getResource() const noexcept { return m_Resource.Get(); }

    DxDescriptor getSrvDescriptor() const noexcept { return m_SrvDescriptor; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadHead;
    DxDescriptor                           m_SrvDescriptor;
    DxDescriptorPool*                      m_SrvDescPool;
};

}  // namespace Engine