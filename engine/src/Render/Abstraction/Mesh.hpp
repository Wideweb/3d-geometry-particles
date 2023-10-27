#pragma once

#include <vector>

#include "Vertex.hpp"

namespace Engine {

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
    Mesh(const std::vector<Vertex> &vertices);
};

} // namespace Engine