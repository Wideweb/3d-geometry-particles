#define STB_IMAGE_IMPLEMENTATION

#include "TextureLoader.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#include "stb_image.hpp"
#pragma GCC diagnostic pop

#include <cassert>
#include <iostream>
#include <vector>

#include "glad/glad.h"

namespace Engine {

Texture TextureLoader::m_Placeholder;

Texture TextureLoader::loadTexture(const std::string& path) {
    Texture texture;
    int     width, height, channels;

    stbi_set_flip_vertically_on_load(0);

    unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);

    assert(data && "file not found.");

    GLenum internalFormat = 0, dataFormat = 0;
    if (channels == 4) {
        internalFormat = GL_RGBA8;
        dataFormat     = GL_RGBA;
    } else if (channels == 3) {
        internalFormat = GL_RGB8;
        dataFormat     = GL_RGB;
    } else if (channels == 1) {
        internalFormat = GL_R8;
        dataFormat     = GL_RED;
    }

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    GLint mipmapLevel = 0;
    GLint border      = 0;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // clang-format off
    glTexImage2D(GL_TEXTURE_2D,    // Specifies the target texture of the active texture unit
                 mipmapLevel,      // Specifies the level-of-detail number. Level 0 is the base image level
                 internalFormat,   // Specifies the internal format of the texture
                 static_cast<GLsizei>(width),
                 static_cast<GLsizei>(height),
                 border,           // Specifies the width of the border. Must be 0. For GLES 2.0
                 dataFormat,       // Specifies the format of the texel data. Must match internalformat
                 GL_UNSIGNED_BYTE, // Specifies the data type of the texel data
                 data);            // Specifies a pointer to the image data in memory
    // clang-format on

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    texture.width  = width;
    texture.height = height;
    texture.type   = Texture::TextureType::COLOR;

    if (internalFormat == GL_RGBA8) {
        texture.format     = Texture::InternalFormat::RGBA8F;
        texture.dataFormat = Texture::DataFormat::RGBA;
    } else if (internalFormat == GL_RGB8) {
        texture.format     = Texture::InternalFormat::RGB8F;
        texture.dataFormat = Texture::DataFormat::RGB;
    } else if (internalFormat == GL_R8) {
        texture.format     = Texture::InternalFormat::R8F;
        texture.dataFormat = Texture::DataFormat::RED;
    }
    texture.dataType = Texture::DataType::UNSIGNED_BYTE;

    return texture;
}

}  // namespace Engine
