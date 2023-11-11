#include "Model.hpp"

#include "glad/glad.h"

namespace Engine {

Model::Model() {}

Model::Model(const std::vector<Mesh>& meshes)
: meshes(meshes) {}

void Model::setUp() {
    for (auto& mesh : meshes) {
        mesh.setUp();
    }
}

void Model::update() {
    for (auto& mesh : meshes) {
        mesh.update();
    }
}

void Model::draw() {
    for (const auto& mesh : meshes) {
        mesh.draw();
    }
}

}  // namespace Engine
