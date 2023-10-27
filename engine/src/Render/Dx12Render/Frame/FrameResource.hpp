#pragma once

#include "DxUtils.hpp"
#include <memory>

#include "UploadBuffer.hpp"

struct FrameResource {
    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;

    uint64_t fence = 0;

    FrameResource(ID3D12Device* device, size_t capacity) {
        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(cmdListAlloc.GetAddressOf())
        ));

        mainPassData = std::make_unique<UploadBuffer<MainPassData>>(device, 1);
    }
}
