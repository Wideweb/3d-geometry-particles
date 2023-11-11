#pragma once

#include <wincodec.h>

#include <array>
#include <string>

#include "DxUtils.hpp"

namespace Engine {

class DxTextureLoader {
public:
    static Microsoft::WRL::ComPtr<ID3D12Resource> loadTextureFromFile(
        ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& filename
    );

    static void loadImageDataFromFile(
        ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& filename,
        Microsoft::WRL::ComPtr<ID3D12Resource>& textureResource,
        Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap
    );

    static void loadCubeMapDataFromFile(
        ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::array<std::string, 6>& files,
        Microsoft::WRL::ComPtr<ID3D12Resource>& textureResource,
        Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap
    );

private:
    static int loadImageDataFromFile(
        BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow
    );
    static DXGI_FORMAT        getDXGIFormatFromWICFormat(WICPixelFormatGUID& wic_format_guid);
    static WICPixelFormatGUID getConvertToWICFormat(WICPixelFormatGUID& wic_format_guid);
    static int                getDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgi_format);
};

}  // namespace Engine