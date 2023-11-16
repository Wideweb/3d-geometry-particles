#include "DxReadWriteDataBuffer.hpp"

namespace Engine {

DxReadWriteDataBuffer::DxReadWriteDataBuffer(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList, size_t byteSize
)
: m_CommandList(commandList) {
    auto bufferProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc       = CD3DX12_RESOURCE_DESC::Buffer(byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    ThrowIfFailed(device->CreateCommittedResource(
        &bufferProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(&m_Buffer)
    ));

    auto readBackBufferProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    auto readBackBufferDesc       = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &readBackBufferProperties, D3D12_HEAP_FLAG_NONE, &readBackBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(&m_ReadBackBuffer)
    ));

    auto writeBufferProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto writeBufferDesc       = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &writeBufferProperties, D3D12_HEAP_FLAG_NONE, &writeBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_WriteBuffer)
    ));

    ThrowIfFailed(m_ReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_ReadMappedData)));
    ThrowIfFailed(m_WriteBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_WriteMappedData)));

    m_ByteSize = byteSize;
}

DxReadWriteDataBuffer::~DxReadWriteDataBuffer() {
    if (m_ReadBackBuffer != nullptr) {
        m_ReadBackBuffer->Unmap(0, nullptr);
    }

    m_Buffer          = nullptr;
    m_ReadBackBuffer  = nullptr;
    m_ReadMappedData  = nullptr;
    m_WriteMappedData = nullptr;
}

void DxReadWriteDataBuffer::copyData(void* data) { memcpy(m_WriteMappedData, data, m_ByteSize); }

void DxReadWriteDataBuffer::copyData(void* data, size_t byteSize) { memcpy(m_WriteMappedData, data, byteSize); }

void DxReadWriteDataBuffer::readFromVRAM() {
    // Schedule to copy the data to the default buffer to the readback

    auto toCopySourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_Buffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
    );
    m_CommandList->ResourceBarrier(1, &toCopySourceBarrier);

    m_CommandList->CopyResource(m_ReadBackBuffer.Get(), m_Buffer.Get());

    auto toCommonBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );
    m_CommandList->ResourceBarrier(1, &toCommonBarrier);
}

void DxReadWriteDataBuffer::writeToVRAM() {
    auto toCopyDestBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_Buffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST
    );
    m_CommandList->ResourceBarrier(1, &toCopyDestBarrier);

    m_CommandList->CopyResource(m_Buffer.Get(), m_WriteBuffer.Get());

    auto toCommonBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );
    m_CommandList->ResourceBarrier(1, &toCommonBarrier);
}

}  // namespace Engine