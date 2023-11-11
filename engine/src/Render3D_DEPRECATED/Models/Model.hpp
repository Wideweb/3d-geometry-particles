#pragma once

#include <unordered_map>
#include <vector>

#include "Mesh.hpp"

namespace Engine {

class Model {
public:
    std::vector<Mesh> meshes;

    Model();
    Model(const std::vector<Mesh>& meshes);

    void setUp();
    void update();
    void draw();
};

}  // namespace Engine