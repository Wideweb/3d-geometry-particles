#include "FrameResource.hpp"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount) {
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(cmdListAlloc.GetAddressOf())
    ));

    passCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    objectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);


    UINT objCBByteSize = DxUtils::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT objCount = (UINT)m_RenderItems.size();

    // Need a CBV descriptor for each object for each frame resource.
    for (int frameIndex = 0; frameIndex < c_NumFrameResources; frameIndex++) {
        auto objectCB = m_FrameResources[frameIndex]->objectCB->Resource();
        for (UINT i = 0; i < objCount; i++) {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

            // Offset to the ith object constant buffer in the buffer.
            cbAddress += i * objCBByteSize;

            // Offset to the object cbv in the descriptor heap.
            int heapIndex = frameIndex * objCount + i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = cbAddress;
            cbvDesc.SizeInBytes = objCBByteSize;

            m_D3dDevice->CreateConstantBufferView(&cbvDesc, handle);
        }
    }

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

    // Last three descriptors are the pass CBVs for each frame resource.
    for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++) {
        auto passCB = mFrameResources[frameIndex]->passCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

        // Offset to the pass cbv in the descriptor heap.
        int heapIndex = m_PassCbvOffset + frameIndex;
        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = cbAddress;
        cbvDesc.SizeInBytes = passCBByteSize;
        
        m_D3dDevice->CreateConstantBufferView(&cbvDesc, handle);
    }
}

FrameResource::~FrameResource() { }