#pragma once

#include "DxUtils.hpp"

#include <string>

namespace Engine {

class DxTexture {
public:
	DxTexture(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

	ID3D12Resource* getResource() const noexcept { return m_Resource.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource>              m_Resource;
	std::string 										m_Name;
};

} // namespace Engine