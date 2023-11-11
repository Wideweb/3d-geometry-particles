#pragma once

#include <string>
#include <unordered_map>

#include "DxUtils.h"

namespace Engine {

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index
// buffers so that we can implement the technique described by Figure 6.3.
struct OpenGLSubmeshGeometry {
    GLsizei indexCount          = 0;
    GLint   startIndexLocation  = 0;
    GLint   startVertexLocation = 0;

    // Bounding box of the geometry defined by this submesh.
    // This is used in later chapters of the book.
    // DirectX::BoundingBox bounds;
};

struct OpenGLMeshGeometry {
    // Give it a name so we can look it up by name.
    std::string name;
    GLuint      VAO;
    GLuint      VBO;
    GLuint      EBO;

    // Data about the buffers.
    GLuint vertexByteStride     = 0;
    GLuint vertexBufferByteSize = 0;
    GLuint indexBufferByteSize  = 0;

    // A MeshGeometry may store multiple geometries in one vertex/index buffer.
    // Use this container to define the Submesh geometries so we can draw
    // the Submeshes individually.
    std::unordered_map<std::string, SubmeshGeometry> drawArgs;
};

}  // namespace Engine