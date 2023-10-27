#include <wrl/client.h>
#include <d3d12.h>
#include <string>

struct Texture {
	std::string name;
	std::string filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap = nullptr;
};