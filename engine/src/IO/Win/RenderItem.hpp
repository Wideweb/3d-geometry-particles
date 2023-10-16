#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include "MeshGeometry.hpp"

struct RenderItem {
	RenderItem() = default;

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    glm::mat4 world;

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int numFramesDirty = 3;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT objCBIndex = -1;

	MeshGeometry* geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT indexCount = 0;
    UINT startIndexLocation = 0;
    int baseVertexLocation = 0;
};