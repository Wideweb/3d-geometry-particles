#include "Geometry.hpp"

Geometry::Geometry(const Engine::Mesh& mesh)
: mesh(mesh) {
    int triangles = mesh.vertices.size() / 3;
    for (int i = 0; i < triangles; i++) {
        glm::vec3 P0 = mesh.vertices[i * 3].position;
        glm::vec3 P1 = mesh.vertices[i * 3 + 1].position;
        glm::vec3 P2 = mesh.vertices[i * 3 + 2].position;

        m_VertexToTriangle[P0].push_back(i);
        m_VertexToTriangle[P1].push_back(i);
        m_VertexToTriangle[P2].push_back(i);
    }
}

int Geometry::getAdjacentTriangleByEdge(int triangleIndex, glm::vec3 P0, glm::vec3 P1) {
    std::vector<int>& P0Triangles = m_VertexToTriangle[P0];
    std::vector<int>& P1Triangles = m_VertexToTriangle[P1];

    for (int i0 : P0Triangles) {
        for (int i1 : P1Triangles) {
            if (i0 == i1 && triangleIndex != i0) {
                return i0;
            }
        }
    }

    return -1;
}

const std::vector<int>& Geometry::getTriangles(glm::vec3 vertex) { return m_VertexToTriangle[vertex]; }