#pragma once

#include "glad/glad.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "OpenGLUtils.hpp"
#include "OpenGLMeshGeometry.hpp"
#include "Mesh.hpp"

namespace Engine {

class OpenGLGeometryRegistry {
public:
    const OpenGLMeshGeometry* get(const std::string& geometry) const;

    void add(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes);
  
private:
    std::unordered_map<std::string, std::unique_ptr<OpenGLMeshGeometry>> m_Data;
}

} // namespace Engine