#pragma once

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <limits>

#include "Engine.hpp"

namespace FrustrumCullingGPU {

class Frustrum {
public:
    static void getInWorldSpace(Engine::Camera& camera, std::vector<glm::vec3>& vertices, float farFarctor) {
        int w, h;
        camera.getSize(w, h);
        float fovY = camera.getFieldOfView();

        glm::mat4 cameraWorldMatrix = camera.worldMatrix();

        float s = static_cast<float>(w) / static_cast<float>(h);
        float g = 1.0f / glm::tan(fovY / 2.0f);

        float ak = camera.getZNear();
        float bk = camera.getZFar() * farFarctor;

        float cascadeWidth = bk - ak;

        float ax = ak * s / g;
        float ay = ak / g;

        float bx = bk * s / g;
        float by = bk / g;

        vertices[0] = glm::vec3(cameraWorldMatrix * glm::vec4(ax, ay, -ak, 1.0f));
        vertices[1] = glm::vec3(cameraWorldMatrix * glm::vec4(ax, -ay, -ak, 1.0f));
        vertices[2] = glm::vec3(cameraWorldMatrix * glm::vec4(-ax, -ay, -ak, 1.0f));
        vertices[3] = glm::vec3(cameraWorldMatrix * glm::vec4(-ax, ay, -ak, 1.0f));

        vertices[4] = glm::vec3(cameraWorldMatrix * glm::vec4(bx, by, -bk, 1.0f));
        vertices[5] = glm::vec3(cameraWorldMatrix * glm::vec4(bx, -by, -bk, 1.0f));
        vertices[6] = glm::vec3(cameraWorldMatrix * glm::vec4(-bx, -by, -bk, 1.0f));
        vertices[7] = glm::vec3(cameraWorldMatrix * glm::vec4(-bx, by, -bk, 1.0f));
    }
};

}  // namespace FrustrumCullingGPU