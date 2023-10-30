#pragma once

#include "DxUtils.hpp"

namespace Engine {

namespace DxTextureLoader {
    ID3D12Resource* loadTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& filename);
}

} // namespace Engine