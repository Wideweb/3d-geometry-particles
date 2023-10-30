#include "DxShaderProgramSlot.hpp"

namespace Engine {

DxShaderProgramSlot::DxShaderProgramSlot(ID3D12Device* device, size_t byteSize) {
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_Buffer)
    ));

    ThrowIfFailed(m_Buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));

    m_ByteSize = byteSize;
}

DxShaderProgramSlot::~DxShaderProgramSlot() {
    if (m_Buffer != nullptr) {
        m_Buffer->Unmap(0, nullptr);
    }

    m_MappedData = nullptr;
}

void DxShaderProgramSlot::copyData(void* data) {
    memcpy(&m_MappedData, &data, m_ByteSize);
}

} // namespace Engine