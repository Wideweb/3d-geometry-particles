#include "DescriptorPool.hpp"

#include <vector>
#include <stdexcept>

DescriptorPool::DescriptorPool(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, size_t capacity) :
    m_Device(device),
    m_HeapType(type),
    m_HeapFlags(flags),
    m_Capacity(capacity),
    m_Size(0)
{
    m_DescriptorSize = device->GetDescriptorHandleIncrementSize(type);

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = capacity;
    heapDesc.Type = type;
    heapDesc.Flags = flags;
	heapDesc.NodeMask = 0; // For single-adapter operation, set this to zero.

    ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_Heap.GetAddressOf())));
}

DescriptorPool::~DescriptorPool()
{
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorPool::get()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_Heap->GetCPUDescriptorHandleForHeapStart());

    int offset = m_Size;
    if (m_Free.size() > 0) {
        offset = m_Free[m_Free.size() - 1];
        m_Free.pop_back();
    } else {
        m_Size++;
    }

    if (offset >= m_Capacity) {
        throw std::out_of_range("Too many descriptors requested.");
    }
     
    heapHandle.Offset(offset, m_DescriptorSize);
    return heapHandle;
}

void DescriptorPool::release(CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
{
    m_Free.push_back(handle.ptr / m_DescriptorSize);
}