#include "GeometryParticle.hpp"

#include <cmath>
#include <limits>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

GeometryParticle::GeometryParticle(Engine::Mesh& geometry) : m_Geometry(geometry) {}

void GeometryParticle::setUp() {
    int index0;
    int index1;
    int index2;
    
    if (m_Geometry.indices.size() > 0) {
      m_TriangleIndex = Engine::Math::rand(m_Geometry.indices.size() / 3 - 1);
      
      int i0 = m_TriangleIndex * 3;
      int i1 = i0 + 1;
      int i2 = i0 + 2;

      index0 = m_Geometry.indices.at(i0);
      index1 = m_Geometry.indices.at(i1);
      index2 = m_Geometry.indices.at(i2);
    } else {
      m_TriangleIndex = Engine::Math::rand(m_Geometry.vertices.size() / 3 - 1);

      index0 = m_TriangleIndex * 3;
      index1 = index0 + 1;
      index2 = index0 + 2;
    }

    Engine::Vertex& vertex0 = m_Geometry.vertices.at(index0);
    Engine::Vertex& vertex1 = m_Geometry.vertices.at(index1);
    Engine::Vertex& vertex2 = m_Geometry.vertices.at(index2);

    glm::vec3 P0 = vertex0.position;
    glm::vec3 P1 = vertex1.position;
    glm::vec3 P2 = vertex2.position;

    glm::vec3 N = getTriangleNormal(P0, P1, P2);

    float w0 = Engine::Math::randFloat();
    float w1 = Engine::Math::randFloat() * (1.0f - w0);
    float w2 = 1.0f - w0 - w1;

    glm::vec3 P = P0 * w0 + P1 * w1 + P2 * w2;

    glm::vec3 V = glm::normalize(P0 - P); 
    glm::vec3 Q = glm::cross(V, N);

    float angle = Engine::Math::randFloat() * 2.0f - 1.0f;
    m_Velocity = V * std::cos(angle) + Q * std::sin(angle);
    m_Position = P;
    m_P0 = P0;
    m_P1 = P1;
    m_P2 = P2;
}

void GeometryParticle::update() {
  if (!isInsideTriangle(m_P0, m_P1, m_P2, m_Position + m_Velocity * m_Speed)) {
    moveToNextTriangle();
  }
  m_Position += m_Velocity * m_Speed;
}

bool GeometryParticle::isInsideTriangle(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P) {
    glm::mat3 m = glm::mat3(P0, P1, P2);
    glm::vec3 weights = glm::inverse(m) * P;

    float epsilon = 0.0001f;

    if (weights.x < -epsilon || weights.y < -epsilon || weights.z < -epsilon) {
      return false;
    }

    if (std::abs(weights.x + weights.y + weights.z - 1.0f) > epsilon) {
      return false;
    }

    return true;
}

void GeometryParticle::moveToNextTriangle() {
  int triangle;
  int index0;
  int index1;
  int index2;
  glm::vec3 displacement;

  if (m_Geometry.indices.size() > 0) {
    auto result = findNextIndexedTriangle(m_Geometry);
    triangle = result.first;

    if (triangle < 0) {
      m_Velocity *= -1.0f;
      return;
    }

    int i0 = triangle * 3;
    int i1 = i0 + 1;
    int i2 = i0 + 2;

    index0 = m_Geometry.indices.at(i0);
    index1 = m_Geometry.indices.at(i1);
    index2 = m_Geometry.indices.at(i2);

    displacement = result.second;

  } else {
    auto result = findNextTriangle(m_Geometry);
    triangle = result.first;

    if (triangle < 0) {
      m_Velocity *= -1;
      return;
    }

    index0 = triangle * 3;
    index1 = index0 + 1;
    index2 = index0 + 2;

    displacement = result.second;
  }

  glm::vec3 N0 = getTriangleNormal(m_P0, m_P1, m_P2);

  m_P0 = m_Geometry.vertices[index0].position;
  m_P1 = m_Geometry.vertices[index1].position;
  m_P2 = m_Geometry.vertices[index2].position;

  glm::vec3 N1 = getTriangleNormal(m_P0, m_P1, m_P2);

  m_Velocity = glm::normalize(rotate(N0, N1, m_Velocity));
  m_Position += displacement;
  m_TriangleIndex = triangle;
}

std::pair<int, glm::vec3> GeometryParticle::findNextIndexedTriangle(Engine::Mesh& mesh) {
  glm::vec3 minDisplacement;
  float minDisplacementLength = std::numeric_limits<float>::max();
  int nextTriangleIndex = -1;

  size_t triangles = mesh.indices.size() / 3;
  for (size_t i = 0; i < triangles; i++) {
    if (i == m_TriangleIndex) {
      continue;
    }

    int i0 = mesh.indices[i * 3];
    int i1 = mesh.indices[i * 3 + 1];
    int i2 = mesh.indices[i * 3 + 2];

    glm::vec3 P0 = mesh.vertices[i0].position;
    glm::vec3 P1 = mesh.vertices[i1].position;
    glm::vec3 P2 = mesh.vertices[i2].position;

    auto edge = getCommonEdge(P0, P1, P2);
    if (!edge.has_value()) {
      continue;
    }

    glm::vec3 displacement = getDisplacementToLine(edge.value().first, edge.value().second, m_Position);

    float displacementLength = glm::length(displacement);
    if (minDisplacementLength > displacementLength) {
      minDisplacement = displacement;
      minDisplacementLength = displacementLength;
      nextTriangleIndex = i;
    }
  }

  return std::make_pair(nextTriangleIndex, minDisplacement);
}

std::pair<int, glm::vec3> GeometryParticle::findNextTriangle(Engine::Mesh& mesh) {
  glm::vec3 minDisplacement;
  float minDisplacementLength = std::numeric_limits<float>::max();
  int nextTriangleIndex = -1;

  size_t triangles = mesh.vertices.size() / 3;
  for (size_t i = 0; i < triangles; i++) {
    if (i == m_TriangleIndex) {
      continue;
    }

    glm::vec3 P0 = mesh.vertices[i * 3].position;
    glm::vec3 P1 = mesh.vertices[i * 3 + 1].position;
    glm::vec3 P2 = mesh.vertices[i * 3 + 2].position;

    auto edge = getCommonEdge(P0, P1, P2);
    if (!edge.has_value()) {
      continue;
    }

    glm::vec3 edgeStart = edge.value().first;
    glm::vec3 edgeEnd = edge.value().second;

    if (!isInsideEdgeBounds(edgeStart, edgeEnd, m_Position)) {
      continue;
    }

    glm::vec3 displacement = getDisplacementToLine(edgeStart, edgeEnd, m_Position);

    float displacementLength = glm::length(displacement);
    if (minDisplacementLength > displacementLength) {
      minDisplacement = displacement;
      minDisplacementLength = displacementLength;
      nextTriangleIndex = i;
    }
  }

  return std::make_pair(nextTriangleIndex, minDisplacement);
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
  glm::quat q = glm::normalize(glm::quat(glm::dot(N0, half), glm::cross(N0, half)));
  
  return q * V;
}

glm::vec3 GeometryParticle::getTriangleNormal(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2) {
  glm::vec3 P0P1 = P1 - P0;
  glm::vec3 P0P2 = P2 - P0;
  glm::vec3 N = glm::normalize(glm::cross(P0P1, P0P2));
  return N;
}

std::optional<std::pair<glm::vec3, glm::vec3>> GeometryParticle::getCommonEdge(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2) {
  bool isP0Common = Engine::Math::isEqual(P0, m_P0) || Engine::Math::isEqual(P0, m_P1) || Engine::Math::isEqual(P0, m_P2);
  bool isP1Common = Engine::Math::isEqual(P1, m_P0) || Engine::Math::isEqual(P1, m_P1) || Engine::Math::isEqual(P1, m_P2);
  bool isP2Common = Engine::Math::isEqual(P2, m_P0) || Engine::Math::isEqual(P2, m_P1) || Engine::Math::isEqual(P2, m_P2);

  if (isP0Common && isP1Common) {
    return std::make_pair(P0, P1);
  }

  if (isP1Common && isP2Common) {
    return std::make_pair(P1, P2);
  }

  if (isP2Common && isP0Common) {
    return std::make_pair(P2, P0);
  }

  return std::nullopt;
}

glm::vec3 GeometryParticle::getDisplacementToLine(glm::vec3 P0, glm::vec3 P1, glm::vec3 P) {
  glm::vec3 lineDir = glm::normalize(P1 - P0);
  glm::vec3 R = P - P0;
  glm::vec3 proj = P0 + lineDir * glm::dot(lineDir, R);
  return proj - P;
}

bool GeometryParticle::isInsideEdgeBounds(glm::vec3 P0, glm::vec3 P1, glm::vec3 P) {
  glm::vec3 P0P1 = P1 - P0;
  glm::vec3 P0P = P - P0;
  if (glm::dot(P0P1, P0P) < 0) {
    return false;
  }

  glm::vec3 P1P0 = P0 - P1;
  glm::vec3 P1P = P - P1;
  if (glm::dot(P1P0, P1P) < 0) {
    return false;
  }

  return true;
}

glm::mat4 GeometryParticle::getTransform() {
  glm::vec3 N = getTriangleNormal(m_P0, m_P1, m_P2);

  glm::quat rotation1 = Engine::Math::getRotationBetween(glm::vec3(0.0f, 0.0f, -1.0f), m_Velocity);

  glm::vec3 right = cross(m_Velocity, N);
  glm::vec3 up = cross(right, m_Velocity);

  glm::vec3 newUp = rotation1 * glm::vec3(0.0f, 1.0f, 0.0f);
  glm::quat rotation2 = Engine::Math::getRotationBetween(newUp, N);

  glm::mat4 model = glm::mat4(1.0);
  model = glm::translate(model, m_Position);
  model = model * glm::toMat4(rotation2 * rotation1);
  return model;
}