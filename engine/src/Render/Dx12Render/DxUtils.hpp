#pragma once

#include <D3Dcompiler.h>
#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <windows.h>
#include <wrl.h>
#include <wrl/client.h>

#include <cstring>
#include <fstream>
#include <string>
#include <system_error>

#include "d3dx12.h"

namespace Engine {

class DxUtils {
public:
    static bool IsKeyDown(int vkeyCode);

    static UINT CalcConstantBufferByteSize(UINT byteSize) {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }

    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
    );

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        const std::string& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint,
        const std::string& target
    );

    static size_t bitsPerPixel(DXGI_FORMAT fmt);

    static inline std::wstring AnsiToWString(const std::string& str) {
        WCHAR buffer[512];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
        return std::wstring(buffer);
    }
};

class DxException {
public:
    DxException() = default;
    DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber);

    std::string ToString() const;

    HRESULT     ErrorCode = S_OK;
    std::string FunctionName;
    std::string Filename;
    int         LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                     \
    {                                                        \
        HRESULT hr__ = (x);                                  \
        if (FAILED(hr__)) {                                  \
            throw DxException(hr__, #x, __FILE__, __LINE__); \
        }                                                    \
    }
#endif

struct DxDescriptor {
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu;
};

}  // namespace Engine