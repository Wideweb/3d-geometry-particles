#pragma once

#include "DxUtils.h"

#include <unordered_map>
#include <string>

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct OpenGLSubmeshGeometry {
	size_t indexCount = 0;
	size_t startIndexLocation = 0;
	size_t startVertexLocation = 0;

    // Bounding box of the geometry defined by this submesh. 
    // This is used in later chapters of the book.
	// DirectX::BoundingBox bounds;
};

struct OpenGLMeshGeometry {
	// Give it a name so we can look it up by name.
	std::string name;
	size_t VAO;
	size_t VBO;
	size_t EBO;

    // Data about the buffers.
	UINT vertexByteStride = 0;
	UINT vertexBufferByteSize = 0;
	UINT indexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> drawArgs;
};