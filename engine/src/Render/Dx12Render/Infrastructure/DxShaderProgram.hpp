#pragma once

#include "DxUtils.hpp"
#include "DxShaderProgramSlot.hpp"
#include "DxTexture.hpp"
#include "DxRenderTexture.hpp"

#include <vector>
#include <memory>
#include <string>
#include <array>

namespace Engine {

class DxShaderProgram {
public:
    DxShaderProgram(
        ID3D12Device* device,
        const std::string& vertexFile,
        const std::string& pixelFile,
        const std::vector<size_t>& dataSlots,
        size_t textureSlots);

    void setDataSlot(size_t index, void* data);
    void setTextureSlot(size_t index, D3D12_GPU_DESCRIPTOR_HANDLE srvDescriptor);
    void setTextureSlot(size_t index, std::shared_ptr<DxTexture> renderTexture);
    void setTextureSlot(size_t index, std::shared_ptr<DxRenderTexture> renderTexture);

    void bind(ID3D12GraphicsCommandList* commandList);

    ID3DBlob* getVertexShader() const noexcept { return m_VertexShader.Get(); }
    ID3DBlob* getPixelShader() const noexcept { return m_PixelShader.Get(); }

    ID3D12RootSignature* getRootSignature() const noexcept { return m_RootSignature.Get(); }
    const std::vector<D3D12_INPUT_ELEMENT_DESC>& getInputLayout() const noexcept { return m_InputLayout; }

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> getStaticSamplers();

private:
    ID3D12Device*                                     m_Device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>       m_RootSignature;
    std::vector<D3D12_INPUT_ELEMENT_DESC>             m_InputLayout;
    std::vector<std::unique_ptr<DxShaderProgramSlot>> m_DataSlots;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>          m_TxtureSlots;

    Microsoft::WRL::ComPtr<ID3DBlob>                  m_VertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob>                  m_PixelShader;
};

} // namespace Engine