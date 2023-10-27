#pragma once

#include "DxUtils.hpp"
#include "FrameResource.hpp"

#include <memory>
#include <vector>

class RenderResource {
  public:
    std::vector<std::unique_ptr<FrameResource>> frameResources;

    uint64_t currFrameIndex = 0;

    FrameResource* currFrameResource = nullptr;

    RenderResource(ID3D12Device* device) {
        for (int i = 0; i < 3; i++) {
            frameResources[i] = std::make_unique<FrameResource<TData>>(device);
        }

        currFrameResource = frameResources[0].get();
    }

    void beginFrame(int frameIndex, ID3D12Fence* fence) {
        currFrameResource = frameResources[frameIndex].get();

        // Has the GPU finished processing the commands of the current frame resource?
        // If not, wait until the GPU has completed commands up to this fence point.
        if (currFrameResource->fence != 0 && fence->GetCompletedValue() < currFrameResource->fence) {
            HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
            ThrowIfFailed(fence->SetEventOnCompletion(currFrameResource->fence, eventHandle));
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    void setFence(uint64_t fence) {
        currFrameResource->fence = fence;
    }
}