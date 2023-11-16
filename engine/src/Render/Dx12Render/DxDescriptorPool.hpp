#pragma once

#include <vector>

#include "DxUtils.hpp"

namespace Engine {

class DxDescriptorPool {
public:
    DxDescriptorPool(
        ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, size_t capacity
    );

    DxDescriptorPool(DxDescriptorPool* pool, size_t from, size_t to);

    virtual ~DxDescriptorPool();

    DxDescriptor get();

    void release(CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    void release(CD3DX12_GPU_DESCRIPTOR_HANDLE handle);
    void release(DxDescriptor handle);

    ID3D12DescriptorHeap* getHeap() const noexcept { return m_Heap.Get(); }

    size_t getDescriptorSize() const noexcept { return m_DescriptorSize; }

private:
    ID3D12Device*                                m_Device;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
    D3D12_DESCRIPTOR_HEAP_TYPE                   m_HeapType;
    D3D12_DESCRIPTOR_HEAP_FLAGS                  m_HeapFlags;
    size_t                                       m_DescriptorSize;

    size_t              m_From;
    size_t              m_Capacity;
    size_t              m_Size;
    std::vector<size_t> m_Free;
};

}  // namespace Engine