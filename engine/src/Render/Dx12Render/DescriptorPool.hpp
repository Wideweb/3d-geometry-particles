#pragma once

#include "DxUtils.hpp"

#include <vector>

class DescriptorPool 
{
public:
    DescriptorPool(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, size_t capacity);

    virtual ~DescriptorPool();

    CD3DX12_CPU_DESCRIPTOR_HANDLE get();

    void release(CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    
private:
    ID3D12Device* m_Device;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    D3D12_DESCRIPTOR_HEAP_FLAGS m_HeapFlags;
    size_t m_DescriptorSize;

    size_t m_Capacity;
    size_t m_Size;

    std::vector<int> m_Free;
};