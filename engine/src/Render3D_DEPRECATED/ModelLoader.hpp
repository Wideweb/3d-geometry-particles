#pragma once

#include <string>

#include "Model.hpp"

namespace Engine {

class ModelLoader {
public:
    static std::shared_ptr<Model> loadObj(const std::string& path);
};

}  // namespace Engine
