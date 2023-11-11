#pragma once

#include <memory>
#include <vector>

#include "DxShaderProgram.hpp"
#include "DxUtils.hpp"

namespace Engine {

class DxRenderPass {
public:
    struct PipelineDesc {
        D3D12_CULL_MODE       cullMode        = D3D12_CULL_MODE_BACK;
        D3D12_COMPARISON_FUNC depthFunc       = D3D12_COMPARISON_FUNC_LESS;
        bool                  depthClipEnable = true;
    };

    DxRenderPass(
        ID3D12Device* device, std::shared_ptr<DxShaderProgram> shaderProgram, std::vector<DXGI_FORMAT> rtvFormats,
        DXGI_FORMAT dsvFormat, PipelineDesc pipelineDesc
    );

    void bind(ID3D12GraphicsCommandList* commandList);

    ID3D12PipelineState* resource() { return m_Pso.Get(); }

private:
    std::shared_ptr<DxShaderProgram>            m_ShaderProgram;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_Pso;
};

}  // namespace Engine