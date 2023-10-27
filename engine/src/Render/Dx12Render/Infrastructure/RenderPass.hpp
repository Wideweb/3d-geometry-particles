#pragma once

#include "DxUtils.hpp"
#include "Framebuffer.hpp"
#include "ShaderProgram.hpp"

#include <memory>
#include <array>

class RenderPass {
public:
    RenderPass(
        ID3D12Device* device,
        std::shared_ptr<ShaderProgram> shaderProgram,
        size_t constantBuffersNum,
        size_t texturesNum,
        std::vector<DXGI_FORMAT> rtvFormats,
        DXGI_FORMAT dsvFormat
    );

    void bind(ID3D12GraphicsCommandList* commandList);

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

    Microsoft::WRL::ComPtr<ID3D12PipelineState>  m_Pso;

    static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> getStaticSamplers();
};