#pragma once

#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram(const std::string& vertexFile, const std::string& pixelFile) noexcept {   
    m_VertexShader = DxUtils::CompileShader(vertexFile, nullptr, "VS", "vs_5_1");
    m_PixelShader = DxUtils::CompileShader(pixelFile, nullptr, "PS", "ps_5_1");
}

void ShaderProgram::addAttribute(std::string name, DXGI_FORMAT format) {
    D3D12_INPUT_ELEMENT_DESC desc;
    desc.SemanticName = name.c_str();
    desc.SemanticIndex = 0;
    desc.Format = format;
    desc.InputSlot = 0;
    desc.AlignedByteOffset = m_InputOffset;
    desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    desc.InstanceDataStepRate = 0;

    m_InputLayout.push_back(desc);
    m_InputOffset += DxUtils::bitsPerPixel(format) / 8;
}