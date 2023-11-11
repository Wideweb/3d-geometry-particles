#pragma once

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec3.hpp>

namespace Engine::Math {

const float c_Epsilon = 0.00001f;

glm::vec3 getDirection(glm::vec3 rotation);

inline bool isEqual(float x, float y, float epsilon) { return std::abs(x - y) <= epsilon * std::abs(x); }

inline bool isEqual(float x, float y) { return isEqual(x, y, c_Epsilon); }

inline bool isEqual(glm::vec3 v0, glm::vec3 v1, float epsilon) {
    return isEqual(v0.x, v1.x, epsilon) && isEqual(v0.y, v1.y, epsilon) && isEqual(v0.z, v1.z, epsilon);
}

inline bool isEqual(glm::vec3 v0, glm::vec3 v1) { return isEqual(v0, v1, c_Epsilon); }

inline void srand() { std::srand(static_cast<unsigned>(std::time(0))); }

inline float randFloat() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }

inline float rand(int max) { return static_cast<int>(max * randFloat()); }

inline float rescale(float value, float prevScale, float newScale) {
    if (isEqual(prevScale, 0.0f) || isEqual(value, 0.0f)) {
        return newScale;
    }

    return value / prevScale * newScale;
}

inline glm::vec3 rescale(glm::vec3 value, glm::vec3 prevScale, glm::vec3 newScale) {
    return glm::vec3(
        rescale(value.x, prevScale.x, newScale.x), rescale(value.y, prevScale.y, newScale.y),
        rescale(value.z, prevScale.z, newScale.z)
    );
}

inline glm::vec2 solve(float a, float b, float c) {
    if (isEqual(a, 0.0f)) {
        return glm::vec2(c / b);
    }

    float D = b * b - 4 * a * c;
    if (D < 0.0f) {
        return glm::vec2(0.0f);
    }

    if (isEqual(D, 0.0f)) {
        return glm::vec2(-1.0f * b / (2.0f * a));
    }

    float x1 = (-1.0f * b + std::sqrtf(D)) / (2.0f * a);
    float x2 = (-1.0f * b - std::sqrtf(D)) / (2.0f * a);

    return glm::vec2(x1, x2);
}

inline glm::vec3 moveAlongAxis(glm::vec3 viewPos, glm::vec3 objPos, glm::vec3 ray0, glm::vec3 ray1, glm::vec3 axis) {
    float sin0 = glm::length(glm::cross(ray0, axis));
    float sin1 = glm::length(glm::cross(ray1, axis));

    glm::vec3 distance0 = objPos - viewPos;
    glm::vec3 distance1 = ray1 * glm::length(distance0) / sin1 * sin0;

    return (distance1 - distance0) * axis;
}

inline glm::vec3 orthogonal(glm::vec3 v) {
    float x = abs(v.x);
    float y = abs(v.y);
    float z = abs(v.z);

    glm::vec3 X_AXIS = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 Y_AXIS = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 Z_AXIS = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 other = x < y ? (x < z ? X_AXIS : Z_AXIS) : (y < z ? Y_AXIS : Z_AXIS);
    return glm::cross(v, other);
}

inline glm::quat getRotationBetween(glm::vec3 u, glm::vec3 v) {
    u = glm::normalize(u);
    v = glm::normalize(v);

    // Unfortunately, we have to check for when u == -v, as u + v
    // in this case will be (0, 0, 0), which cannot be normalized.
    if (isEqual(u, -v)) {
        // 180 degree rotation around any orthogonal vector
        return glm::quat(0.0f, glm::normalize(orthogonal(u)));
    }

    glm::vec3 half = glm::normalize(u + v);
    return glm::quat(glm::dot(u, half), glm::cross(u, half));
}

}  // namespace Engine::Math