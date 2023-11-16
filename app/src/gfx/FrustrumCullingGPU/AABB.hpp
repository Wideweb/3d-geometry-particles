#pragma once

#include <algorithm>
#include <glm/vec3.hpp>
#include <limits>
#include <vector>

#include "Engine.hpp"

namespace FrustrumCullingGPU {

struct AABB {
    glm::vec3 min;
    float padding0;
    glm::vec3 max;
    float padding1;

    AABB() {}

    AABB(glm::vec3 min, glm::vec3 max)
    : min(std::move(min)),
      max(std::move(max)) {}

    AABB(const std::vector<glm::vec3> vertices) {
        min = glm::vec3((std::numeric_limits<float>::max)());
        max = glm::vec3((std::numeric_limits<float>::min)());

        for (glm::vec3 v : vertices) {
            min.x = std::min<float>(min.x, v.x);
            min.y = std::min<float>(min.y, v.y);
            min.z = std::min<float>(min.z, v.z);

            max.x = std::max<float>(max.x, v.x);
            max.y = std::max<float>(max.y, v.y);
            max.z = std::max<float>(max.z, v.z);
        }
    }

    AABB(const Engine::Mesh& mesh) {
        min = glm::vec3((std::numeric_limits<float>::max)());
        max = glm::vec3((std::numeric_limits<float>::min)());

        for (unsigned int index : mesh.indices) {
            const Engine::Vertex& v = mesh.vertices[index];

            min.x = std::min<float>(min.x, v.position.x);
            min.y = std::min<float>(min.y, v.position.y);
            min.z = std::min<float>(min.z, v.position.z);

            max.x = std::max<float>(max.x, v.position.x);
            max.y = std::max<float>(max.y, v.position.y);
            max.z = std::max<float>(max.z, v.position.z);
        }
    }

    bool intersect(const AABB& other) const {
        return (
            max.x > other.min.x && min.x < other.max.x && max.y > other.min.y && min.y < other.max.y &&
            max.z > other.min.z && min.z < other.max.z
        );
    }

    AABB move(glm::vec3 d) const { return {min + d, max + d}; }
};

}  // namespace FrustrumCullingGPU