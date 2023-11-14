#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <optional>
#include <utility>

#include "Engine.hpp"
#include "Geometry.hpp"

struct TriangleTransition {
    glm::vec3 from;
    glm::vec3 to;
    int       index;

    TriangleTransition(int index)
    : index(index) {}

    TriangleTransition(int index, glm::vec3 from, glm::vec3 to)
    : index(index),
      from(from),
      to(to) {}
};

class GeometryParticle {
private:
    Geometry& m_Geometry;

    glm::vec3 m_Position;
    glm::vec3 m_Velocity;
    glm::vec3 m_Q;
    glm::vec3 m_P0, m_P1, m_P2;

    float m_Speed = 0.005f;
    int   counter = 0;

    int m_TriangleIndex;

public:
    GeometryParticle(Geometry& geometry);

    void      setUp();
    void      update();
    glm::mat4 getTransform();

    glm::vec3 getRight() const { return m_Q; }

    glm::vec4 color = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);

private:
    bool               isInsideTriangle(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P);
    void               moveToNextTriangle();
    TriangleTransition findNextTriangle();
    TriangleTransition findNextTriangleByVertices();
    TriangleTransition findNextTriangleByEdge();

    glm::vec3                rotate(glm::vec3 N0, glm::vec3 N1, glm::vec3 V);
    glm::vec3                getTriangleNormal(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2);
    std::optional<glm::vec3> getInterceptPoint(glm::vec3 planeNormal, glm::vec3 planePoint, glm::vec3 P0, glm::vec3 P1);
    bool                     isSameInterval(glm::vec3 A0, glm::vec3 A1, glm::vec3 B0, glm::vec3 B1);
};
