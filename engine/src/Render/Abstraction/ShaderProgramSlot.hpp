#pragma once

#include <string>

namespace Engine {

enum class SHADER_PROGRAM_SLOT_TYPE { DATA, DATA_ARRAY, READ_WRITE_DATA, TEXTURE, TEXTURE_ARRAY_4 };

struct ShaderProgramSlotDesc {
    std::string              name;
    SHADER_PROGRAM_SLOT_TYPE type;
};

}  // namespace Engine