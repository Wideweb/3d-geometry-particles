#pragma once

#include <stddef.h>

#include <string>

#include "Texture.hpp"

namespace Engine {

class TextureLoader {
private:
    static Texture m_Placeholder;

public:
    static Texture loadTexture(const std::string& path);
};

}  // namespace Engine
