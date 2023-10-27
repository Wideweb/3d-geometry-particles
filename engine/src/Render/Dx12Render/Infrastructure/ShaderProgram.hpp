#pragma once

#include "DxUtils.hpp"

class ShaderProgram {
public:
    ShaderProgram(const std::string& vertexFile, const std::string& pixelFile) noexcept;

    void addAttribute(std::string name, DXGI_FORMAT format);

    ID3DBlob* getVertexShader() const noexcept { return m_VertexShader.Get(); }
    ID3DBlob* getPixelShader() const noexcept { return m_PixelShader.Get(); }
    const std::vector<D3D12_INPUT_ELEMENT_DESC>& getInputLayout() const noexcept { return m_InputLayout; }


private:
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
    size_t m_InputOffset = 0;

    Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShader;
};