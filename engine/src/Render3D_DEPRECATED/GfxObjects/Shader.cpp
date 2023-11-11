#include "Shader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "glad/glad.h"

namespace Engine {

Shader::Shader() {}

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) { compile(vertexSrc, fragmentSrc); }

void Shader::bind() const { glUseProgram(id); }

void Shader::unbind() const { glUseProgram(0); }

void Shader::free() {
    if (!empty()) {
        glDeleteProgram(id);
        setEmpty();
    }
}

void Shader::compile(const std::string& vertexSrc, const std::string& fragmentSrc) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
    if (vertexShader == 0) {
        return;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (fragmentShader == 0) {
        return;
    }

    id = glCreateProgram();
    if (id == 0) {
        std::cerr << "failed to create gl program";
        return;
    }

    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);

    glLinkProgram(id);

    GLint linked_status = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &linked_status);
    if (linked_status == 0) {
        GLint infoLen = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLen);

        std::vector<char> infoLog(static_cast<size_t>(infoLen));
        glGetProgramInfoLog(id, infoLen, nullptr, infoLog.data());

        std::cerr << "Error linking program:\n" << infoLog.data();
        glDeleteProgram(id);
        return;
    }

    glDetachShader(id, vertexShader);
    glDetachShader(id, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    readUniforms();
}

GLuint Shader::compileShader(unsigned int type, const std::string& source) {
    GLuint shader = glCreateShader(type);

    if (source.find("@") != std::string::npos) {
        std::cout << source << std::endl;
    }

    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        std::vector<char> infoLog(static_cast<size_t>(infoLen));
        glGetShaderInfoLog(shader, infoLen, nullptr, infoLog.data());

        glDeleteShader(shader);

        std::cerr << "Error compiling shader(vertex)\n" << infoLog.data() << "\n";
        std::cerr << source << "\n";
        return 0;
    }

    return shader;
}

void Shader::readUniforms() {
    GLint         count;
    GLint         size;           // size of the variable
    GLenum        type;           // type of the variable (float, vec3 or mat4, etc)
    const GLsizei bufSize = 32;   // maximum name length
    GLchar        name[bufSize];  // variable name in GLSL
    GLsizei       length;         // name length

    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count);
    for (GLuint i = 0; i < count; i++) {
        glGetActiveUniform(id, i, bufSize, &length, &size, &type, name);
        m_Uniforms.add(std::string(name), fromNativeType(type));
    }
}

Shader::Property::Type Shader::fromNativeType(int type) {
    switch (type) {
        case GL_INT:
        case GL_UNSIGNED_INT_SAMPLER_2D:
            return Property::Type::INT1;
        case GL_FLOAT:
            return Property::Type::FLOAT1;
        case GL_FLOAT_VEC2:
            return Property::Type::FLOAT2;
        case GL_FLOAT_VEC3:
            return Property::Type::FLOAT3;
        case GL_FLOAT_VEC4:
            return Property::Type::FLOAT4;
        case GL_FLOAT_MAT4:
            return Property::Type::MATRIX4;
        case GL_SAMPLER_2D:
            return Property::Type::TEXTURE;
        case GL_SAMPLER_CUBE:
            return Property::Type::CUBE_MAP_TEXTURE;
    }
    // GL_BOOL 0x8B56

    std::cerr << "Shader property type is unknown: " << type << "\n";
    return Property::Type::UNKNOWN;
}

void Shader::set(const std::string& name, const Property property) {
    switch (property.type) {
        case Property::Type::INT1:
            setInt(name, property.value.int1);
            break;
        case Property::Type::FLOAT1:
            setFloat(name, property.value.float1);
            break;
        case Property::Type::FLOAT2:
            setFloat2(name, property.value.float2);
            break;
        case Property::Type::FLOAT3:
            setFloat3(name, property.value.float3);
            break;
        case Property::Type::FLOAT4:
            setFloat4(name, property.value.float4);
            break;
        case Property::Type::MATRIX4:
            setMatrix4(name, property.value.matrix4);
            break;
        case Property::Type::TEXTURE:
        case Property::Type::CUBE_MAP_TEXTURE:
            setTexture(name, property.value.texture);
            break;
        default:
            std::cerr << "Cant set unknown shader property"
                      << "\n";
    }
}

void Shader::setInt(const std::string& name, int value) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, float value) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniform1f(location, value);
}

void Shader::setFloat2(const std::string& name, float value1, float value2) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniform2f(location, value1, value2);
}

void Shader::setFloat2(const std::string& name, glm::vec2 value) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniform2f(location, value.x, value.y);
}

void Shader::setFloat3(const std::string& name, float value1, float value2, float value3) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniform3f(location, value1, value2, value3);
}

void Shader::setFloat3(const std::string& name, glm::vec3 value) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::setFloat4(const std::string& name, glm::vec4 value) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::setMatrix4(const std::string& name, const glm::mat4& matrix) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setMatrix2x3(const std::string& name, const std::vector<float>& matrix) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniformMatrix2x3fv(location, 1, GL_FALSE, matrix.data());
}

void Shader::setMatrix2(const std::string& name, const std::vector<float>& matrix) {
    GLint location = getUniformLocation(name);
    if (location == -1)
        return;
    glUniformMatrix2fv(location, 1, GL_FALSE, matrix.data());
}

void Shader::setTexture(const std::string& name, const Texture& texture) { setTexture(name, &texture); }

void Shader::setTexture(const std::string& name, const Texture* texture) {
    if (texture == nullptr) {
        return;
    }

    GLint location = getUniformLocation(name);
    if (location == -1) {
        return;
    }

    unsigned int index = 0;
    auto         it    = m_TextureIndex.find(name);
    if (it != m_TextureIndex.end()) {
        index = it->second;
    } else {
        index = m_TextureIndex.size();
        m_TextureIndex.emplace(name, index);
    }
    glActiveTexture(GL_TEXTURE0 + index);
    texture->bind();
    glUniform1i(location, index);
}

int Shader::getUniformLocation(const std::string& name) {
    auto it = m_UniformLocationMap.find(name);
    if (it != m_UniformLocationMap.end()) {
        return it->second;
    }
    GLint location = glGetUniformLocation(id, name.c_str());
    if (location != -1) {
        m_UniformLocationMap[name] = location;
    }
    return location;
}

}  // namespace Engine
