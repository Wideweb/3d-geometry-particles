#include "ModelLoader.hpp"

#include <fstream>
#include <glm/vec3.hpp>
#include <iostream>
#include <numeric>
#include <sstream>
#include <utility>

namespace Engine {

Mesh ModelLoader::loadObj(const std::string& path) {
    std::ifstream     in(path, std::ios::in | std::ios::binary);
    std::stringstream dto;
    std::string       line;

    std::string            attribute;
    std::vector<glm::vec3> pVertices;
    std::vector<glm::vec3> nVertices;
    std::vector<glm::vec2> tVertices;
    std::vector<Vertex>    vertices;

    while (!in.eof() && in >> attribute) {
        if (attribute == "o") {
        } else if (attribute == "v") {
            float x, y, z;
            in >> x >> y >> z;
            pVertices.emplace_back(x, y, z);
        } else if (attribute == "vt") {
            float x, y;
            in >> x >> y;
            tVertices.emplace_back(x, y);
        } else if (attribute == "vn") {
            float x, y, z;
            in >> x >> y >> z;
            nVertices.emplace_back(x, y, z);
        } else if (attribute == "f") {
            char   divider;
            size_t p, t, n;
            in >> p >> divider >> t >> divider >> n;
            vertices.emplace_back(
                pVertices[p - 1], nVertices[n - 1], tVertices[t - 1], glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f)
            );
            in >> p >> divider >> t >> divider >> n;
            vertices.emplace_back(
                pVertices[p - 1], nVertices[n - 1], tVertices[t - 1], glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f)
            );
            in >> p >> divider >> t >> divider >> n;
            vertices.emplace_back(
                pVertices[p - 1], nVertices[n - 1], tVertices[t - 1], glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f)
            );
        } else {
            std::getline(in, line);
        }
    }

    in.close();

    std::vector<unsigned int> indices(vertices.size());
    std::iota(indices.begin(), indices.end(), 0);

    Mesh mesh(vertices, indices);
    return mesh;
}

}  // namespace Engine
