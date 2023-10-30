#pragma once

#include "DxUtils.hpp"

namespace Engine {

class DxShaderProgramSlot {
public:
    DxShaderProgramSlot(ID3D12Device* device, size_t byteSize);

    DxShaderProgramSlot(const DxShaderProgramSlot& rhs) = delete;
    DxShaderProgramSlot& operator=(const DxShaderProgramSlot& rhs) = delete;

    ~DxShaderProgramSlot();

    void copyData(void* data);

    ID3D12Resource* resource() const noexcept { return m_Buffer.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource>    m_Buffer;
    BYTE*                                     m_MappedData;
    size_t                                    m_ByteSize;
};

} // namespace Engine