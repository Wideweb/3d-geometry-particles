#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include "DxUtils.hpp"
#include "Vertex.hpp"

struct ObjectConstants {
    glm::mat4 world;
};

struct PassConstants {
    glm::mat4 view;
    glm::mat4 invView;
    glm::mat4 proj;
    glm::mat4 invProj;
    glm::mat4 viewProj;
    glm::mat4 invViewProj;
    glm::vec3 eyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    glm::vec2 renderTargetSize = { 0.0f, 0.0f };
    glm::vec2 invRenderTargetSize = { 0.0f, 0.0f };
    float nearZ = 0.0f;
    float farZ = 0.0f;
    float totalTime = 0.0f;
    float deltaTime = 0.0f;
};

template<typename TItemData, typename TPassData>
struct FrameResource {
    FrameResource(ID3D12Device* device, uint32_t passCount, uint32_t itemsCount)  {
        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(cmdListAlloc.GetAddressOf())
        ));

        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
        cbvHeapDesc.NumDescriptors = itemsCount + 1;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.NodeMask = 0;
        ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));

        passCB = std::make_unique<UploadBuffer<TPassData>>(device, passCount, true);
        itemCB = std::make_unique<UploadBuffer<TItemData>>(device, itemsCount, true);

        uint32_t itemCBByteSize = DxUtils::CalcConstantBufferByteSize(sizeof(TItemData));

        // Need a CBV descriptor for each object for frame.
        for (uint32_t i = 0; i < itemsCount; i++) {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress = itemCB->GetGPUVirtualAddress();

            // Offset to the ith object constant buffer in the buffer.
            cbAddress += i * itemCBByteSize;

            // Offset to the object cbv in the descriptor heap.
            int heapIndex = itemsCount + i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = cbAddress;
            cbvDesc.SizeInBytes = itemCBByteSize;

            m_D3dDevice->CreateConstantBufferView(&cbvDesc, handle);
        }

        uint32_t passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(TPassData));

        // Last three descriptors are the pass CBVs for each frame resource.
        auto passCB = passCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

        // Offset to the pass cbv in the descriptor heap.
        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(itemsCount, mCbvSrvUavDescriptorSize);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = cbAddress;
        cbvDesc.SizeInBytes = passCBByteSize;
        
        m_D3dDevice->CreateConstantBufferView(&cbvDesc, handle);
    }

    FrameResource(const FrameResource& rhs) = delete;
    
    FrameResource& operator=(const FrameResource& rhs) = delete;
    
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.
    std::unique_ptr<UploadBuffer<TPassData>> passCB = nullptr;
    std::unique_ptr<UploadBuffer<TItemData>> itemCB = nullptr;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvHeap;

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 fence = 0;

    int maxSize = 0;
    int size = 0;

    std::vector<int> freeSlots;
};