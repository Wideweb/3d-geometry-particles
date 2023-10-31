#pragma once

#include "DxUtils.hpp"
#include "DxShaderProgram.hpp"

#include <memory>
#include <array>

namespace Engine {

class DxRenderPass {
public:
    DxRenderPass(
        ID3D12Device* device,
        std::shared_ptr<DxShaderProgram> shaderProgram,
        std::vector<DXGI_FORMAT> rtvFormats,
        DXGI_FORMAT dsvFormat
    );

    void bind(ID3D12GraphicsCommandList* commandList);

private:
    std::shared_ptr<DxShaderProgram>               m_ShaderProgram;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>    m_Pso;
};

} // namespace Engine