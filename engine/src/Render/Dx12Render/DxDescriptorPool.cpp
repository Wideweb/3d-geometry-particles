#include "DxDescriptorPool.hpp"

#include <stdexcept>
#include <vector>

namespace Engine {

DxDescriptorPool::DxDescriptorPool(
    ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, size_t capacity
)
: m_Device(device),
  m_HeapType(type),
  m_HeapFlags(flags),
  m_From(0),
  m_Capacity(capacity),
  m_Size(0) {
    m_DescriptorSize = device->GetDescriptorHandleIncrementSize(type);

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = capacity;
    heapDesc.Type           = type;
    heapDesc.Flags          = flags;
    heapDesc.NodeMask       = 0;  // For single-adapter operation, set this to zero.

    ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_Heap.GetAddressOf())));
}

DxDescriptorPool::DxDescriptorPool(DxDescriptorPool* pool, size_t from, size_t to)
: m_Device(pool->m_Device),
  m_HeapType(pool->m_HeapType),
  m_HeapFlags(pool->m_HeapFlags),
  m_Heap(pool->m_Heap),
  m_From(from),
  m_Capacity(to - from + 1),
  m_Size(0),
  m_DescriptorSize(pool->m_DescriptorSize) {}

DxDescriptorPool::~DxDescriptorPool() {}

DxDescriptor DxDescriptorPool::get() {
    CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandleCPU(m_Heap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE heapHandleGPU(m_Heap->GetGPUDescriptorHandleForHeapStart());

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

    heapHandleCPU.Offset(offset + m_From, m_DescriptorSize);
    heapHandleGPU.Offset(offset + m_From, m_DescriptorSize);

    return {heapHandleCPU, heapHandleGPU};
}

void DxDescriptorPool::release(CD3DX12_CPU_DESCRIPTOR_HANDLE handle) {
    m_Free.push_back((handle.ptr - m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize - m_From);
}

void DxDescriptorPool::release(CD3DX12_GPU_DESCRIPTOR_HANDLE handle) {
    m_Free.push_back((handle.ptr - m_Heap->GetGPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize - m_From);
}

void DxDescriptorPool::release(DxDescriptor handle) { release(handle.cpu); }

}  // namespace Engine