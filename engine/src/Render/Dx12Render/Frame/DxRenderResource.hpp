#pragma once

#include <array>
#include <memory>

#include "DxFrameResource.hpp"
#include "DxUtils.hpp"

namespace Engine {

class DxRenderResource {
public:
    std::array<std::unique_ptr<DxFrameResource>, 3> frameResources;

    uint64_t currFrameIndex = 0;

    DxFrameResource* currFrameResource = nullptr;

    DxRenderResource(ID3D12Device* device) {
        for (int i = 0; i < 3; i++) {
            frameResources[i] = std::make_unique<DxFrameResource>(device);
        }

        currFrameResource = frameResources[0].get();
    }

    void beginFrame(int frameIndex, ID3D12Fence* fence) {
        currFrameResource = frameResources[frameIndex].get();

        // Has the GPU finished processing the commands of the current frame resource?
        // If not, wait until the GPU has completed commands up to this fence point.
        if (currFrameResource->fence != 0 && fence->GetCompletedValue() < currFrameResource->fence) {
            HANDLE eventHandle = CreateEventEx(nullptr, "FRAME", false, EVENT_ALL_ACCESS);
            ThrowIfFailed(fence->SetEventOnCompletion(currFrameResource->fence, eventHandle));
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    void setFence(uint64_t fence) { currFrameResource->fence = fence; }
};

}  // namespace Engine