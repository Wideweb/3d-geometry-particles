#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "DxDepthStencilTexture.hpp"
#include "DxReadWriteDataBuffer.hpp"
#include "DxRenderTexture.hpp"
#include "DxShaderProgramDataBuffer.hpp"
#include "DxTexture.hpp"
#include "DxUtils.hpp"
#include "ShaderProgramSlot.hpp"

namespace Engine {

class DxComputeProgram {
public:
    DxComputeProgram(ID3D12Device* device, const std::string& file, const std::vector<ShaderProgramSlotDesc>& slots);

    void setDataSlot(size_t index, std::shared_ptr<DxShaderProgramDataBuffer> buffer);
    void setDataArraySlot(size_t index, std::shared_ptr<DxShaderProgramDataBuffer> buffer);
    void setReadWriteDataSlot(size_t index, std::shared_ptr<DxReadWriteDataBuffer> buffer);
    void setTextureSlot(size_t index, std::shared_ptr<DxTexture> renderTexture);
    void setTextureSlot(size_t index, std::shared_ptr<DxRenderTexture> renderTexture);
    void setTextureSlot(size_t index, std::shared_ptr<DxDepthStencilTexture> renderTexture);

    void bind(ID3D12GraphicsCommandList* commandList);

    ID3DBlob*            getComputeShader() const noexcept { return m_ComputeShader.Get(); }
    ID3D12RootSignature* getRootSignature() const noexcept { return m_RootSignature.Get(); }

private:
    ID3D12Device*                               m_Device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    std::vector<ShaderProgramSlotDesc>          m_Slots;

    Microsoft::WRL::ComPtr<ID3DBlob> m_ComputeShader;

    ID3D12GraphicsCommandList*               m_CommandList;
    std::vector<std::shared_ptr<DxResource>> m_Resources;
};

}  // namespace Engine