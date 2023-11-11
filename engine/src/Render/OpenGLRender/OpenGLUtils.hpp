#pragma once

#include "glad/glad.h"

namespace Engine::OpenGLUtils {

GLenum checkError_(const char* file, int line);

#define checkError() checkError_(__FILE__, __LINE__)

GLuint compileShader(const std::string& fileName, GLenum type);

}  // namespace Engine::OpenGLUtils