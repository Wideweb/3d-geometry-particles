#include "Mesh.hpp"

#include <iostream>

#include "glad/glad.h"

namespace Engine {

Mesh::Mesh() {}
Mesh::~Mesh() {}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
: vertices(vertices),
  indices(indices) {}

Mesh::Mesh(const std::vector<Vertex>& vertices)
: vertices(vertices) {}

void Mesh::setUp() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * static_cast<GLuint>(indices.size()), indices.data(), GL_STATIC_DRAW
    );

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW
    );

    /////////////////////////////////////////////////////////////
    ///////////////////////// POSITION //////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    /////////////////////////////////////////////////////////////
    ////////////////////////// NORMAL ///////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    /////////////////////////////////////////////////////////////
    ////////////////////// TEXTURE COORD ////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    /////////////////////////////////////////////////////////////
    ///////////////////////// TANGENT ///////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    /////////////////////////////////////////////////////////////
    //////////////////////// BITANGENT //////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    /////////////////////////////////////////////////////////////
    ////////////////////////// COLOR ////////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(14 * sizeof(float)));
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void Mesh::update() {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * static_cast<GLuint>(indices.size()), indices.data(), GL_STATIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::draw() const {
    glBindVertexArray(VAO);

    if (indices.size() > 0) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    }

    glBindVertexArray(0);
}

}  // namespace Engine
