#pragma once

#include "DxUtils.hpp"
#include "DxResource.hpp"

namespace Engine {

class DxShaderProgramDataBuffer : public DxResource {
public:
    DxShaderProgramDataBuffer(ID3D12Device* device, size_t byteSize);

    DxShaderProgramDataBuffer(const DxShaderProgramDataBuffer& rhs) = delete;
    DxShaderProgramDataBuffer& operator=(const DxShaderProgramDataBuffer& rhs) = delete;

    ~DxShaderProgramDataBuffer();

    void copyData(void* data);

    ID3D12Resource* resource() const noexcept { return m_Buffer.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource>    m_Buffer;
    BYTE*                                     m_MappedData;
    size_t                                    m_ByteSize;
};

} // namespace Engine