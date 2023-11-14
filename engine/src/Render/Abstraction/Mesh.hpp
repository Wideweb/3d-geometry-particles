#pragma once

#include <vector>

#include "Vertex.hpp"

namespace Engine {

struct Mesh {
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;

    Mesh() {}
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : vertices(vertices),
      indices(indices) {}
    Mesh(const std::vector<Vertex>& vertices)
    : vertices(vertices) {}
};

}  // namespace Engine
