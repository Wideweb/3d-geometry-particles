#pragma once

#include <memory>

#include "DxUtils.hpp"
#include "UploadBuffer.hpp"

namespace Engine {

struct DxFrameResource {
    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;

    uint64_t fence = 0;

    DxFrameResource(ID3D12Device* device) {
        ThrowIfFailed(
            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAlloc.GetAddressOf()))
        );
    }
};

}  // namespace Engine