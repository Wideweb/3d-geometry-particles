#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <vector>

#include "Engine.hpp"

class Geometry {
public:
    Engine::Mesh mesh;

    Geometry(const Engine::Mesh& mesh);

    int                     getAdjacentTriangleByEdge(int triangleIndex, glm::vec3 P0, glm::vec3 P1);
    const std::vector<int>& getTriangles(glm::vec3 vertex);

private:
    std::unordered_map<glm::vec3, std::vector<int>> m_VertexToTriangle;
};
