#include "GeometryParticle.hpp"

#include <cmath>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <limits>

GeometryParticle::GeometryParticle(Geometry& geometry)
: m_Geometry(geometry) {}

void GeometryParticle::setUp() {
    Engine::Mesh& mesh = m_Geometry.mesh;

    Engine::Math::srand();

    m_TriangleIndex = Engine::Math::rand(mesh.vertices.size() / 3 - 1);

    int index0 = m_TriangleIndex * 3;
    int index1 = index0 + 1;
    int index2 = index0 + 2;

    Engine::Vertex& vertex0 = mesh.vertices.at(index0);
    Engine::Vertex& vertex1 = mesh.vertices.at(index1);
    Engine::Vertex& vertex2 = mesh.vertices.at(index2);

    glm::vec3 P0 = vertex0.position;
    glm::vec3 P1 = vertex1.position;
    glm::vec3 P2 = vertex2.position;

    glm::vec3 N = getTriangleNormal(P0, P1, P2);

    // float a = glm::dot(N, prep_N);
     
    float w0 = Engine::Math::randFloat() + 0.3f;
    float w1 = Engine::Math::randFloat() + 0.3f;
    float w2 = Engine::Math::randFloat() + 0.3f;

    float s = w0 + w1 + w2;

    w0 /= s;
    w1 /= s;
    w2 /= s;

    glm::vec3 P = P0 * w0 + P1 * w1 + P2 * w2;

    glm::vec3 V = glm::normalize(P1 - P);
    glm::vec3 Q = glm::normalize(glm::cross(V, N));

    float angle = Engine::Math::randFloat() * 3.14f;
    m_Q         = V * std::cos(angle) + Q * std::sin(angle);

    // float angle = Engine::Math::randFloat() * 6.28f;
    m_Position = P;
    // m_Velocity = glm::normalize(P1 + glm::vec3(0.0f, 0.0f, 0.0f) - m_Position); //V * std::cos(angle) + Q *
    // std::sin(angle);
    m_P0 = P0;
    m_P1 = P1;
    m_P2 = P2;

    // float angleX = Engine::Math::randFloat() * 1.57f;
    // float angleY = Engine::Math::randFloat() * 0.57f + 0.78f;
    // float angleZ = Engine::Math::randFloat() * 0.57f + 0.78f;

    // glm::mat4 radnomRotate = glm::mat4(1.0f);
    // radnomRotate = glm::rotate(radnomRotate, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
    // radnomRotate = glm::rotate(radnomRotate, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    // radnomRotate = glm::rotate(radnomRotate, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));

    // glm::vec4 rotatedNormal = glm::normalize(radnomRotate * glm::vec4(N, 1.0f));
    // m_Q = glm::vec3(rotatedNormal.x, rotatedNormal.y, rotatedNormal.z);
    // m_Q = glm::cross(m_Velocity, N);
    // m_Q = glm::vec3(1.0f, 0.0f, 0.0f);
    // m_Q = prep_N;

    std::optional<glm::vec3> ip01 = getInterceptPoint(m_Q, m_Position, m_P0, m_P1);
    std::optional<glm::vec3> ip12 = getInterceptPoint(m_Q, m_Position, m_P1, m_P2);
    std::optional<glm::vec3> ip20 = getInterceptPoint(m_Q, m_Position, m_P2, m_P0);

    glm::vec3 edgeInterceptPoint;
    if (ip01.has_value() && glm::distance(ip01.value(), m_Position) > m_Speed) {
        edgeInterceptPoint = ip01.value();
    } else if (ip12.has_value() && glm::distance(ip12.value(), m_Position) > m_Speed) {
        edgeInterceptPoint = ip12.value();
    } else if (ip20.has_value() && glm::distance(ip20.value(), m_Position) > m_Speed) {
        edgeInterceptPoint = ip20.value();
    }

    m_Velocity = glm::normalize(m_Position - edgeInterceptPoint);
    // float t = Engine::Math::randFloat();
    // m_Speed = 0.005 + (0.02 - 0.005) * t;
}

void GeometryParticle::update() {
    if (!isInsideTriangle(m_P0, m_P1, m_P2, m_Position)) {
        moveToNextTriangle();
    }

    m_Position += m_Velocity * m_Speed;

    // glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), 0.001f, glm::vec3(1.0f, 0.0f, 0.0f));
    // m_Q = glm::normalize(rotate * glm::vec4(m_Q, 1.0f));
}

bool GeometryParticle::isInsideTriangle(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P) {
    float epsilon = 0.0001f;

    glm::vec3 u = P1 - P0;
    glm::vec3 v = P2 - P0;
    glm::vec3 w = P - P0;

    glm::vec3 n    = glm::cross(u, v);
    float     area = glm::dot(n, n);

    float w2 = glm::dot(glm::cross(u, w), n) / area;
    if (w2 < -epsilon) {
        return false;
    }

    float w1 = glm::dot(glm::cross(w, v), n) / area;
    if (w1 < -epsilon) {
        return false;
    }

    float w0 = 1.0f - w1 - w2;
    if (w0 < -epsilon) {
        return false;
    }

    return true;
}

void GeometryParticle::moveToNextTriangle() {
    auto result = findNextTriangle();

    if (result.index < 0) {
        m_Velocity *= -1;
        return;
    }

    m_P0 = m_Geometry.mesh.vertices[result.index * 3].position;
    m_P1 = m_Geometry.mesh.vertices[result.index * 3 + 1].position;
    m_P2 = m_Geometry.mesh.vertices[result.index * 3 + 2].position;

    m_Velocity = glm::normalize(result.to - result.from);
    m_Position = result.from;

    m_TriangleIndex = result.index;
}

TriangleTransition GeometryParticle::findNextTriangle() {
    auto result = findNextTriangleByVertices();
    if (result.index >= 0) {
        return result;
    }

    result = findNextTriangleByEdge();
    if (result.index < 0) {
        // throw 1;
        m_Speed = 0.0f;
        color   = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }

    return result;
}

TriangleTransition GeometryParticle::findNextTriangleByVertices() {
    int       nextTriangleIndex = -1;
    glm::vec3 wayPoint          = glm::vec3(0.0f);
    glm::vec3 nextWayPoint      = glm::vec3(0.0f);

    float d0 = glm::dot(m_Velocity, glm::normalize(m_P0 - m_Position));
    float d1 = glm::dot(m_Velocity, glm::normalize(m_P1 - m_Position));
    float d2 = glm::dot(m_Velocity, glm::normalize(m_P2 - m_Position));

    float dist0 = glm::distance(m_P0, m_Position);
    float dist1 = glm::distance(m_P1, m_Position);
    float dist2 = glm::distance(m_P2, m_Position);

    // check move direction to vertex
    if (dist0 < m_Speed) {
        wayPoint = m_P0;
    } else if (dist1 < m_Speed) {
        wayPoint = m_P1;
    } else if (dist2 < m_Speed) {
        wayPoint = m_P2;
    } else if (d0 > 0.999f) {
        wayPoint = m_P0;
    } else if (d1 > 0.999f) {
        wayPoint = m_P1;
    } else if (d2 > 0.999f) {
        wayPoint = m_P2;
    } else {
        return {-1};
    }

    bool      movedByEdge = false;
    glm::vec3 prevWayPoint;
    if (d0 < -0.999f && !Engine::Math::isEqual(m_P0, wayPoint)) {
        prevWayPoint = m_P0;
        movedByEdge  = true;
    } else if (d1 < -0.999f && !Engine::Math::isEqual(m_P1, wayPoint)) {
        prevWayPoint = m_P1;
        movedByEdge  = true;
    } else if (d2 < -0.999f && !Engine::Math::isEqual(m_P2, wayPoint)) {
        prevWayPoint = m_P2;
        movedByEdge  = true;
    }

    if (movedByEdge) {
        int adjacentTriangle = m_Geometry.getAdjacentTriangleByEdge(m_TriangleIndex, prevWayPoint, wayPoint);

        glm::vec3 P0 = m_Geometry.mesh.vertices[adjacentTriangle * 3].position;
        glm::vec3 P1 = m_Geometry.mesh.vertices[adjacentTriangle * 3 + 1].position;
        glm::vec3 P2 = m_Geometry.mesh.vertices[adjacentTriangle * 3 + 2].position;

        glm::vec3 N0 = getTriangleNormal(m_P0, m_P1, m_P2);
        glm::vec3 N1 = getTriangleNormal(P0, P1, P2);

        glm::vec3 N = glm::normalize(N0 + N1);
        m_Q         = glm::cross(N, m_Velocity);
    }

    std::vector<int> triangles = m_Geometry.getTriangles(wayPoint);
    for (int i : triangles) {
        if (i == m_TriangleIndex) {
            continue;
        }

        glm::vec3 P0 = m_Geometry.mesh.vertices[i * 3].position;
        glm::vec3 P1 = m_Geometry.mesh.vertices[i * 3 + 1].position;
        glm::vec3 P2 = m_Geometry.mesh.vertices[i * 3 + 2].position;

        glm::vec3 opposite0;
        glm::vec3 opposite1;

        // is triangle adjacent by vertex
        if (P0 == wayPoint) {
            opposite0 = P1;
            opposite1 = P2;
        } else if (P1 == wayPoint) {
            opposite0 = P2;
            opposite1 = P0;
        } else if (P2 == wayPoint) {
            opposite0 = P0;
            opposite1 = P1;
        } else {
            continue;
        }

        // if (movedByEdge && Engine::Math::isEqual(prevWayPoint, opposite0)) {
        //   nextTriangleIndex = i;
        //   nextWayPoint = opposite0;
        // }

        // if (movedByEdge && Engine::Math::isEqual(prevWayPoint, opposite1)) {
        //   nextTriangleIndex = i;
        //   nextWayPoint = opposite1;
        //   continue;
        // }

        // glm::vec3 N = getTriangleNormal(m_P0, m_P1, m_P2);
        // glm::vec3 Q = glm::cross(m_Velocity, N);

        // is found triangle's edge lies on move plane
        float distance0 = std::abs(glm::dot(m_Q, opposite0 - wayPoint));
        if (distance0 < 0.001) {
            return {i, wayPoint, opposite0};
        }

        float distance1 = std::abs(glm::dot(m_Q, opposite1 - wayPoint));
        if (distance1 < 0.001) {
            return {i, wayPoint, opposite1};
        }

        // find intersection of oposide triangle's edge and move plane
        bool sign0 = std::signbit(glm::dot((opposite0 - wayPoint), m_Q));
        bool sign1 = std::signbit(glm::dot((opposite1 - wayPoint), m_Q));
        if (sign0 != sign1) {
            std::optional<glm::vec3> ip = getInterceptPoint(m_Q, wayPoint, opposite0, opposite1);
            if (ip.has_value()) {
                return {i, wayPoint, ip.value()};
            }
        }
    }

    return {nextTriangleIndex, wayPoint, nextWayPoint};
}

TriangleTransition GeometryParticle::findNextTriangleByEdge() {
    std::optional<glm::vec3> ip01 = getInterceptPoint(m_Q, m_Position, m_P0, m_P1);
    std::optional<glm::vec3> ip12 = getInterceptPoint(m_Q, m_Position, m_P1, m_P2);
    std::optional<glm::vec3> ip20 = getInterceptPoint(m_Q, m_Position, m_P2, m_P0);

    glm::vec3 edgeStartPoint;
    glm::vec3 edgeEndPoint;
    glm::vec3 edgeInterceptPoint;

    float dist = (std::numeric_limits<float>::max)();

    if (ip01.has_value()) {
        float d01 = glm::distance(ip01.value(), m_Position);
        if (d01 < dist) {
            edgeStartPoint     = m_P0;
            edgeEndPoint       = m_P1;
            edgeInterceptPoint = ip01.value();
            dist               = d01;
        }
    }

    if (ip12.has_value()) {
        float d12 = glm::distance(ip12.value(), m_Position);
        if (d12 < dist) {
            edgeStartPoint     = m_P1;
            edgeEndPoint       = m_P2;
            edgeInterceptPoint = ip12.value();
            dist               = d12;
        }
    }

    if (ip20.has_value()) {
        float d20 = glm::distance(ip20.value(), m_Position);
        if (d20 < dist) {
            edgeStartPoint     = m_P2;
            edgeEndPoint       = m_P0;
            edgeInterceptPoint = ip20.value();
            dist               = d20;
        }
    }

    int i = m_Geometry.getAdjacentTriangleByEdge(m_TriangleIndex, edgeStartPoint, edgeEndPoint);

    if (i < 0) {
        glm::vec3 N = glm::normalize(glm::cross(m_P1 - m_P0, m_P2 - m_P0));
        glm::vec3 R = glm::normalize(glm::cross(edgeEndPoint - edgeStartPoint, N));
        glm::vec3 Q = m_Q - 2.0f * (glm::dot(R, m_Q) * R);
        m_Q         = glm::normalize(Q);
        i           = m_TriangleIndex;
    }

    glm::vec3 tP0 = m_Geometry.mesh.vertices[i * 3].position;
    glm::vec3 tP1 = m_Geometry.mesh.vertices[i * 3 + 1].position;
    glm::vec3 tP2 = m_Geometry.mesh.vertices[i * 3 + 2].position;

    std::optional<glm::vec3> ip = std::nullopt;

    // glm::vec3 N = getTriangleNormal(tP0, tP1, tP2);
    // m_Q = glm::cross(m_Velocity, N);

    if (!isSameInterval(edgeStartPoint, edgeEndPoint, tP0, tP1)) {
        ip = getInterceptPoint(m_Q, edgeInterceptPoint, tP0, tP1);
    }

    if (!ip.has_value() && !isSameInterval(edgeStartPoint, edgeEndPoint, tP1, tP2)) {
        ip = getInterceptPoint(m_Q, edgeInterceptPoint, tP1, tP2);
    }

    if (!ip.has_value() && !isSameInterval(edgeStartPoint, edgeEndPoint, tP2, tP0)) {
        ip = getInterceptPoint(m_Q, edgeInterceptPoint, tP2, tP0);
    }

    if (ip.has_value()) {
        return {i, edgeInterceptPoint, ip.value()};
    }

    return {-1};
}

bool GeometryParticle::isSameInterval(glm::vec3 A0, glm::vec3 A1, glm::vec3 B0, glm::vec3 B1) {
    if (!Engine::Math::isEqual(A0, B0) && !Engine::Math::isEqual(A0, B1)) {
        return false;
    }

    if (!Engine::Math::isEqual(A1, B0) && !Engine::Math::isEqual(A1, B1)) {
        return false;
    }

    return true;
}

glm::vec3 GeometryParticle::rotate(glm::vec3 N0, glm::vec3 N1, glm::vec3 V) {
    float dot = glm::dot(N0, N1);
    if (dot > 0.999f) {
        return V;
    }

    if (dot < -0.999f) {
        return V * -1.0f;
    }

    glm::vec3 half = glm::normalize(N0 + N1);
    glm::quat q    = glm::normalize(glm::quat(glm::dot(N0, half), glm::cross(N0, half)));

    return q * V;
}

glm::vec3 GeometryParticle::getTriangleNormal(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2) {
    glm::vec3 P0P1 = P1 - P0;
    glm::vec3 P0P2 = P2 - P0;
    glm::vec3 N    = glm::normalize(glm::cross(P0P1, P0P2));
    return N;
}

std::optional<glm::vec3> GeometryParticle::getInterceptPoint(
    glm::vec3 planeNormal, glm::vec3 planePoint, glm::vec3 P0, glm::vec3 P1
) {
    float dist = glm::dot(planeNormal, planePoint - P0);
    float t    = dist / glm::dot(P1 - P0, planeNormal);

    if (t < -0.001 || t > 1.001) {
        return std::nullopt;
    }

    glm::vec3 Pt = P0 + (P1 - P0) * t;
    return std::optional<glm::vec3>(Pt);
}

glm::mat4 GeometryParticle::getTransform() {
    glm::vec3 N = getTriangleNormal(m_P0, m_P1, m_P2);

    glm::quat rotation1 = Engine::Math::getRotationBetween(glm::vec3(0.0f, 0.0f, -1.0f), m_Velocity);

    glm::vec3 right = cross(m_Velocity, N);
    glm::vec3 up    = cross(right, m_Velocity);

    glm::vec3 newUp     = rotation1 * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::quat rotation2 = Engine::Math::getRotationBetween(newUp, N);

    glm::mat4 model = glm::mat4(1.0);
    model           = glm::translate(model, m_Position);
    model           = model * glm::toMat4(rotation2 * rotation1);
    return model;
}