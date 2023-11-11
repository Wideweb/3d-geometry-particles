#include "OpenGLUtils.hpp"

#include <cstring>
#include <iostream>
#include <vector>

#include "File.hpp"

namespace Engine::OpenGLUtils {

GLenum checkError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

GLuint compileShader(const std::string& fileName, GLenum type) {
    GLuint shader = glCreateShader(type);

    std::string source     = File::read(fileName);
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

}  // namespace Engine::OpenGLUtils