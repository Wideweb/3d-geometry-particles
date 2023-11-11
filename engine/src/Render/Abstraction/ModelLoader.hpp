#pragma once

#include <string>

#include "Mesh.hpp"

namespace Engine {

class ModelLoader {
public:
    static Mesh loadObj(const std::string& path);
};

}  // namespace Engine
