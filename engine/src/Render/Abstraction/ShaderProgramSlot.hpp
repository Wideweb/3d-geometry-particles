#pragma once

#include <string>

namespace Engine {

enum class SHADER_PROGRAM_SLOT_TYPE { 
    DATA,
    TEXTURE
 };

struct ShaderProgramSlotDesc {
    std::string name;
    SHADER_PROGRAM_SLOT_TYPE type;
};

} // namespace Engine