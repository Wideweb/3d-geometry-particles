#pragma once

#include <algorithm>
#include <vector>

#include "Engine.hpp"
#include "TBN.hpp"

class ModelFactory {
public:
    static Engine::Mesh createCubeIndexed() {
        std::vector<Engine::Vertex> vertices;

        vertices.emplace_back(
            glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        vertices.emplace_back(
            glm::vec3(-1.0f, +1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        vertices.emplace_back(
            glm::vec3(+1.0f, +1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        vertices.emplace_back(
            glm::vec3(+1.0f, -1.0f, -1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        vertices.emplace_back(
            glm::vec3(-1.0f, -1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        vertices.emplace_back(
            glm::vec3(-1.0f, +1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        vertices.emplace_back(
            glm::vec3(+1.0f, +1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        vertices.emplace_back(
            glm::vec3(+1.0f, -1.0f, +1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );

        std::vector<unsigned int> indices = {// front face
                                             0, 1, 2, 0, 2, 3,
                                             // back face
                                             4, 6, 5, 4, 7, 6,
                                             // left face
                                             4, 5, 1, 4, 1, 0,
                                             // right face
                                             3, 2, 6, 3, 6, 7,
                                             // top face
                                             1, 5, 6, 1, 6, 2,
                                             // bottom face
                                             4, 0, 3, 4, 3, 7
        };

        TBN::calculate(vertices, indices);

        Engine::Mesh mesh(vertices, indices);
        return mesh;
    }

    static Engine::Mesh createCube(float left, float right, float bottom, float top, float back, float front) {
        // clang-format off
        std::vector<glm::vec3> positions = {
            glm::vec3(-left, top, front), // 0
            glm::vec3(-left, -bottom, front), // 1
            glm::vec3(-left, top, -back), // 2
            glm::vec3(-left, -bottom, -back), // 3
            glm::vec3(right, top, front), // 4
            glm::vec3(right, -bottom, front), // 5
            glm::vec3(right, top, -back), // 6
            glm::vec3(right, -bottom, -back), // 7
        };

        std::vector<Engine::Vertex> vertices;
        vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        
        vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        
        vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        
        vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[6], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[5], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[7], glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[2], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[3], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));

        vertices.emplace_back(positions[4], glm::vec3(0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[0], glm::vec3(0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        vertices.emplace_back(positions[1], glm::vec3(0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.8f, 0.6f, 0.1f));
        // clang-format on

        std::vector<unsigned int> indices(vertices.size());
        std::iota(indices.begin(), indices.end(), 0);

        TBN::calculate(vertices);

        Engine::Mesh mesh(vertices, indices);
        return mesh;
    }

    static Engine::Mesh createPlane(float tileSize, int columns, int rows) {
        // clang-format off
        glm::vec2 center;
        center.x = columns * tileSize / 2.0f;
        center.y = rows * tileSize / 2.0f;

        std::vector<Engine::Vertex> vertices;
        std::vector<unsigned int> indices;

        for (int i = 0; i <= rows; i++) {
            for (int j = 0; j <= columns; j++) {
                float x = static_cast<float>(j);
                float z = static_cast<float>(i);

                vertices.emplace_back(
                    glm::vec3(x * tileSize - center.x, z * tileSize - center.y, 0.0f), 
                    glm::vec3(0.0f),
                    glm::vec2(x, z),
                    glm::vec3(0.0f),
                    glm::vec3(0.0f),
                    glm::vec3(0.8f, 0.6f, 0.1f)
                );

                if (i != rows && j != columns) {
                    unsigned int v1 = (i + 1) * (columns + 1) + j;
                    unsigned int v2 = i * (columns + 1) + j + 1;
                    unsigned int v3 = i * (columns + 1) + j;

                    indices.push_back(v1);
                    indices.push_back(v2);
                    indices.push_back(v3);

                    v1 = (i + 1) * (columns + 1) + j;
                    v2 = (i + 1) * (columns + 1) + j + 1;
                    v3 = i * (columns + 1) + j + 1;

                    indices.push_back(v1);
                    indices.push_back(v2);
                    indices.push_back(v3);
                }
            }
        }
        // clang-format on

        TBN::calculate(vertices, indices);

        Engine::Mesh mesh(vertices, indices);
        return mesh;
    }

    static Engine::Mesh createSphere(float radius, size_t sliceCount, size_t stackCount) {
        Engine::Mesh mesh;

        Engine::Vertex topVertex(
            glm::vec3(0.0f, radius, 0.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );
        Engine::Vertex bottomVertex(
            glm::vec3(0.0f, -radius, 0.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f),
            glm::vec3(0.8f, 0.6f, 0.1f)
        );

        mesh.vertices.push_back(topVertex);

        float pi = 3.14159265358979323846;

        float phiStep   = pi / stackCount;
        float thetaStep = 2.0f * pi / sliceCount;

        // Compute vertices for each stack ring (do not count the poles as rings).
        for (size_t i = 1; i <= stackCount - 1; i++) {
            float phi = i * phiStep;

            // Vertices of ring.
            for (size_t j = 0; j <= sliceCount; j++) {
                float theta = j * thetaStep;

                Engine::Vertex v;

                // spherical to cartesian
                v.position.x = radius * sinf(phi) * cosf(theta);
                v.position.y = radius * cosf(phi);
                v.position.z = radius * sinf(phi) * sinf(theta);

                v.normal = v.position;

                v.textCoord.x = theta / pi;
                v.textCoord.y = phi / pi;

                mesh.vertices.push_back(v);
            }
        }

        mesh.vertices.push_back(bottomVertex);

        //
        // Compute indices for top stack.  The top stack was written first to the vertex buffer
        // and connects the top pole to the first ring.
        //

        for (size_t i = 1; i <= sliceCount; ++i) {
            mesh.indices.push_back(0);
            mesh.indices.push_back(i + 1);
            mesh.indices.push_back(i);
        }

        //
        // Compute indices for inner stacks (not connected to poles).
        //

        // Offset the indices to the index of the first vertex in the first ring.
        // This is just skipping the top pole vertex.
        size_t baseIndex       = 1;
        size_t ringVertexCount = sliceCount + 1;
        for (size_t i = 0; i < stackCount - 2; ++i) {
            for (size_t j = 0; j < sliceCount; ++j) {
                mesh.indices.push_back(baseIndex + i * ringVertexCount + j);
                mesh.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

                mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
                mesh.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
            }
        }

        //
        // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
        // and connects the bottom pole to the bottom ring.
        //

        // South pole vertex was added last.
        size_t southPoleIndex = mesh.vertices.size() - 1;

        // Offset the indices to the index of the first vertex in the last ring.
        baseIndex = southPoleIndex - ringVertexCount;

        for (size_t i = 0; i < sliceCount; ++i) {
            mesh.indices.push_back(southPoleIndex);
            mesh.indices.push_back(baseIndex + i);
            mesh.indices.push_back(baseIndex + i + 1);
        }

        return mesh;
    }
};