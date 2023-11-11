#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Mesh.hpp"
#include "OpenGLMeshGeometry.hpp"
#include "OpenGLUtils.hpp"
#include "glad/glad.h"

namespace Engine {

class OpenGLGeometryRegistry {
public:
    const OpenGLMeshGeometry* get(const std::string& geometry) const;

    void add(
        const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
    );

private:
    std::unordered_map<std::string, std::unique_ptr<OpenGLMeshGeometry>> m_Data;
}

}  // namespace Engine