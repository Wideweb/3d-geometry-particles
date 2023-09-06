#pragma once

#include "Texture.hpp"

#include <stddef.h>
#include <string>

namespace Engine {

class TextureLoader {
  private:
    static Texture m_Placeholder;

  public:
    static Texture loadTexture(const std::string &path);
};

} // namespace Engine
