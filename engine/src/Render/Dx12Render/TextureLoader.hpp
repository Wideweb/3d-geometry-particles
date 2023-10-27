#pragma once

#include "DxUtils.hpp"

namespace TextureLoader {
    ID3D12Resource* LoadTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& filename);
}