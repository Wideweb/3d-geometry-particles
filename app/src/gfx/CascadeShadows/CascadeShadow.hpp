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

class CascadeShadow {
public:
    struct Cascade {
        glm::mat4 viewProj;
        glm::vec4 frontPlane;
    };

    static std::array<Cascade, 4> calculate(Engine::Camera& camera, const glm::mat4& lightView, std::array<float, 8> distances, float zMinOffset, float zMaxOffset) {
        int w, h;
        camera.getSize(w, h);
        float fovY = camera.getFieldOfView();

        float s = static_cast<float>(w) / static_cast<float>(h);
        float g = 1.0f / glm::tan(fovY / 2.0f);

        glm::mat4              cameraWorldMatrix = camera.worldMatrix();
        glm::mat4              L                 = lightView * cameraWorldMatrix;
        std::array<Cascade, 4> cascades = {};

        for (size_t k = 0; k < distances.size() / 2; k++) {
            float ak = distances[k * 2];
            float bk = distances[k * 2 + 1];

            float cascadeWidth = bk - ak;

            float ax = ak * s / g;
            float ay = ak / g;

            float bx = bk * s / g;
            float by = bk / g;

            std::array<glm::vec4, 8> vertices;

            vertices[0] = glm::vec4(ax, ay, -ak, 1.0f);
            vertices[1] = glm::vec4(ax, -ay, -ak, 1.0f);
            vertices[2] = glm::vec4(-ax, -ay, -ak, 1.0f);
            vertices[3] = glm::vec4(-ax, ay, -ak, 1.0f);

            vertices[4] = glm::vec4(bx, by, -bk, 1.0f);
            vertices[5] = glm::vec4(bx, -by, -bk, 1.0f);
            vertices[6] = glm::vec4(-bx, -by, -bk, 1.0f);
            vertices[7] = glm::vec4(-bx, by, -bk, 1.0f);

            float d06 = glm::distance(vertices[0], vertices[6]);
            float d46 = glm::distance(vertices[4], vertices[6]);

            float dk = std::ceil(std::max<float>(d06, d46));
            float T  = dk / cascadeWidth;

            vertices[0] = L * vertices[0];
            vertices[1] = L * vertices[1];
            vertices[2] = L * vertices[2];
            vertices[3] = L * vertices[3];
            vertices[4] = L * vertices[4];
            vertices[5] = L * vertices[5];
            vertices[6] = L * vertices[6];
            vertices[7] = L * vertices[7];

            float xMax = std::max_element(vertices.begin(), vertices.end(), [](glm::vec3 lhs, glm::vec3 rhs) {
                             return lhs.x < rhs.x;
                         })->x;

            float xMin = std::max_element(vertices.begin(), vertices.end(), [](glm::vec3 lhs, glm::vec3 rhs) {
                             return lhs.x > rhs.x;
                         })->x;

            float yMax = std::max_element(vertices.begin(), vertices.end(), [](glm::vec3 lhs, glm::vec3 rhs) {
                             return lhs.y < rhs.y;
                         })->y;

            float yMin = std::max_element(vertices.begin(), vertices.end(), [](glm::vec3 lhs, glm::vec3 rhs) {
                             return lhs.y > rhs.y;
                         })->y;

            float zMax = std::max_element(vertices.begin(), vertices.end(), [](glm::vec3 lhs, glm::vec3 rhs) {
                             return lhs.z < rhs.z;
                         })->z + zMaxOffset;

            float zMin = std::max_element(vertices.begin(), vertices.end(), [](glm::vec3 lhs, glm::vec3 rhs) {
                             return lhs.z > rhs.z;
                         })->z + zMinOffset;

            float sk_x = ((xMax + xMin) / (2 * T)) * T;
            float sk_y = ((yMax + yMin) / (2 * T)) * T;
            float sk_z = zMin;

            glm::vec3 sk = glm::vec3(sk_x, sk_y, sk_z);

            glm::mat4 Mk_inv = glm::mat4(
                lightView[0][0], lightView[0][1], lightView[0][2], 0.0f,
                lightView[1][0], lightView[1][1], lightView[1][2], 0.0f,
                lightView[2][0], lightView[2][1], lightView[2][2], 0.0f,
                -sk_x, -sk_y, -sk_z, 1.0f
            );

            glm::mat4 Pk = glm::mat4(
                2.0f / dk, 0.0f, 0.0f, 0.0f,
                0.0f, 2.0f / dk, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f / (zMax - zMin), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );

            cascades[k].viewProj = Pk * Mk_inv;

            if (k > 0) {
                float     bk_prev = distances[k * 2 - 1];
                glm::vec3 n       = glm::vec3(cameraWorldMatrix[2]) * -1.0f;
                glm::vec3 c       = glm::vec3(cameraWorldMatrix[3]);

                glm::vec4 fk = glm::vec4(n, -glm::dot(n, c) - ak);
                fk /= (bk_prev - ak);

                cascades[k].frontPlane = fk;
            }
        }

        return cascades;
    }
};