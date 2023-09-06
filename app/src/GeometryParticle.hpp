#pragma once

#include "Engine.hpp"

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <optional>
#include <utility>

class GeometryParticle {
  private:
    Engine::Shader m_Shader;
    Engine::Mesh& m_Geometry;

    glm::vec3 m_Position;
    glm::vec3 m_Velocity;
    glm::vec3 m_P0, m_P1, m_P2;

    float m_Speed = 0.005f;

    int m_TriangleIndex;

  public:
    GeometryParticle(Engine::Mesh& geometry);

    void setUp();
    void update();
    glm::mat4 getTransform();

  private:
    bool isInsideTriangle(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P);
    void moveToNextTriangle();
    std::pair<int, glm::vec3> findNextTriangle(Engine::Mesh& mesh);
    std::pair<int, glm::vec3> findNextIndexedTriangle(Engine::Mesh& mesh);
    glm::vec3 rotate(glm::vec3 N0, glm::vec3 N1, glm::vec3 V);
    glm::vec3 getTriangleNormal(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2);
    std::optional<std::pair<glm::vec3, glm::vec3>> getCommonEdge(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2);
    glm::vec3 getDisplacementToLine(glm::vec3 P0, glm::vec3 P1, glm::vec3 P);
    bool isInsideEdgeBounds(glm::vec3 P0, glm::vec3 P1, glm::vec3 P);
};
