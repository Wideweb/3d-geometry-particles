#pragma once

#include "DxUtils.h"

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry {
	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	INT baseVertexLocation = 0;

    // Bounding box of the geometry defined by this submesh. 
    // This is used in later chapters of the book.
	// DirectX::BoundingBox bounds;
};

struct MeshGeometry {
	// Give it a name so we can look it up by name.
	std::string Name;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU  = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

    // Data about the buffers.
	UINT vertexByteStride = 0;
	UINT vertexBufferByteSize = 0;
	DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
	UINT indexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> drawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const {
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = vertexByteStride;
		vbv.SizeInBytes = vertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const {
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = indexBufferByteSize;

		return ibv;
	}

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders() {
		vertexBufferUploader = nullptr;
		indexBufferUploader = nullptr;
	}
};