#pragma once

#include <glm/glm.hpp>

namespace Engine {

#pragma pack(push, 1)
struct Vertex {
    glm::vec3 position  = glm::vec3(0.0f);
    glm::vec3 normal    = glm::vec3(0.0f);
    glm::vec2 textCoord = glm::vec3(0.0f);
    glm::vec3 tangent   = glm::vec3(0.0f);
    glm::vec3 bitangent = glm::vec3(0.0f);
    glm::vec3 color     = glm::vec3(0.0f);

    Vertex() {}
    Vertex(
        glm::vec3 position, glm::vec3 normal, glm::vec2 textCoord, glm::vec3 tangent, glm::vec3 bitangent,
        glm::vec3 color
    )
    : position(std::move(position)),
      normal(std::move(normal)),
      textCoord(std::move(textCoord)),
      tangent(std::move(tangent)),
      bitangent(std::move(bitangent)),
      color(std::move(color)) {}

    Vertex& operator+=(const Vertex& rhs) {
        this->position += rhs.position;
        this->normal += rhs.normal;
        this->textCoord += rhs.textCoord;
        this->tangent += rhs.tangent;
        this->bitangent += rhs.bitangent;
        this->color += rhs.color;

        return *this;
    };

    Vertex operator/(float div) {
        return Vertex(
            this->position / div, this->normal / div, this->textCoord / div, this->tangent / div, this->bitangent / div,
            this->color / div
        );
    };

    Vertex operator*(float mul) {
        return Vertex(
            this->position * mul, this->normal * mul, this->textCoord * mul, this->tangent * mul, this->bitangent * mul,
            this->color * mul
        );
    };

    Vertex operator/(float div) const {
        return Vertex(
            this->position / div, this->normal / div, this->textCoord / div, this->tangent / div, this->bitangent / div,
            this->color / div
        );
    };

    Vertex operator*(float mul) const {
        return Vertex(
            this->position * mul, this->normal * mul, this->textCoord * mul, this->tangent * mul, this->bitangent * mul,
            this->color * mul
        );
    };
};
#pragma pack(pop)

}  // namespace Engine