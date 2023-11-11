#include "DxShaderProgramDataBuffer.hpp"

namespace Engine {

DxShaderProgramDataBuffer::DxShaderProgramDataBuffer(ID3D12Device* device, size_t byteSize) {
    auto bufferProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    size_t byteSizeAligned = DxUtils::CalcConstantBufferByteSize(byteSize);
    auto   bufferDesc      = CD3DX12_RESOURCE_DESC::Buffer(byteSizeAligned);

    ThrowIfFailed(device->CreateCommittedResource(
        &bufferProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_Buffer)
    ));

    ThrowIfFailed(m_Buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));

    m_ByteSize = byteSize;
}

DxShaderProgramDataBuffer::~DxShaderProgramDataBuffer() {
    if (m_Buffer != nullptr) {
        m_Buffer->Unmap(0, nullptr);
    }

    m_Buffer     = nullptr;
    m_MappedData = nullptr;
}

void DxShaderProgramDataBuffer::copyData(void* data) { memcpy(m_MappedData, data, m_ByteSize); }

}  // namespace Engine