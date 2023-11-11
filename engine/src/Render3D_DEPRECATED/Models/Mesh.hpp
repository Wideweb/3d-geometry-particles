#pragma once

#include <memory>
#include <vector>

#include "Vertex.hpp"

namespace Engine {

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    Mesh(const std::vector<Vertex>& vertices);

    Mesh(const Mesh& mesh);

    Mesh();
    ~Mesh();

    void draw() const;

    void setUp();
    void update();

public:
    unsigned int VAO, VBO, EBO;
};

}  // namespace Engine
