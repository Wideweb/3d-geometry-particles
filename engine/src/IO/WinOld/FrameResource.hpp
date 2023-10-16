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

struct FrameResource {
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);

    FrameResource(const FrameResource& rhs) = delete;
    
    FrameResource& operator=(const FrameResource& rhs) = delete;
    
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.
    std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB = nullptr;

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 fence = 0;
};