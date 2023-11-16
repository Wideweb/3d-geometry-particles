#pragma once

#include "DxResource.hpp"
#include "DxUtils.hpp"

namespace Engine {

class DxReadWriteDataBuffer : public DxResource {
public:
    DxReadWriteDataBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, size_t byteSize);

    DxReadWriteDataBuffer(const DxReadWriteDataBuffer& rhs)            = delete;
    DxReadWriteDataBuffer& operator=(const DxReadWriteDataBuffer& rhs) = delete;

    ~DxReadWriteDataBuffer();

    void copyData(void* data);
    void copyData(void* data, size_t byteSize);
    void readFromVRAM();
    void writeToVRAM();

    void* data() { return m_ReadMappedData; }

    ID3D12Resource* resource() const noexcept { return m_Buffer.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_ReadBackBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_WriteBuffer;
    BYTE*                                  m_ReadMappedData;
    BYTE*                                  m_WriteMappedData;
    size_t                                 m_ByteSize;
    ID3D12GraphicsCommandList*             m_CommandList;
};

}  // namespace Engine