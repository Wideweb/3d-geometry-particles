#include "ModelLoader.hpp"

#include <fstream>
#include <glm/vec3.hpp>
#include <iostream>
#include <sstream>
#include <utility>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#include "stb_image.hpp"
#pragma GCC diagnostic pop

namespace Engine {

std::shared_ptr<Model> ModelLoader::loadObj(const std::string& path) {
    std::ifstream     in(path, std::ios::in | std::ios::binary);
    std::stringstream dto;
    std::string       line;

    std::string               attribute;
    std::vector<glm::vec3>    pVertices;
    std::vector<glm::vec3>    nVertices;
    std::vector<glm::vec2>    tVertices;
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;

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

    Mesh mesh(vertices, indices);
    auto model = std::shared_ptr<Model>(new Model({mesh}));
    model->setUp();
    return model;
}

}  // namespace Engine
