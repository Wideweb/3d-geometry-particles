#include "DxRenderPass.hpp"

namespace Engine {

DxRenderPass::DxRenderPass(
    ID3D12Device* device, std::shared_ptr<DxShaderProgram> shaderProgram, std::vector<DXGI_FORMAT> rtvFormats,
    DXGI_FORMAT dsvFormat, PipelineDesc pipelineDesc
) {
    //////////////////// Pipeline State Objects ////////////////////
    CD3DX12_RASTERIZER_DESC rasterDesc(D3D12_DEFAULT);
    rasterDesc.FillMode              = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode              = pipelineDesc.cullMode;
    rasterDesc.FrontCounterClockwise = true;
    rasterDesc.DepthClipEnable       = pipelineDesc.depthClipEnable;

    // rasterDesc.DepthBias = 100000;
    // rasterDesc.DepthBiasClamp = 0.0f;
    // rasterDesc.SlopeScaledDepthBias = 1.0f;

    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthFunc = pipelineDesc.depthFunc;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout    = {shaderProgram->getInputLayout().data(), (UINT)shaderProgram->getInputLayout().size()};
    psoDesc.pRootSignature = shaderProgram->getRootSignature();
    psoDesc.VS             = {
        reinterpret_cast<BYTE*>(shaderProgram->getVertexShader()->GetBufferPointer()),
        shaderProgram->getVertexShader()->GetBufferSize()
    };

    if (shaderProgram->getPixelShader() != nullptr) {
        psoDesc.PS = {
            reinterpret_cast<BYTE*>(shaderProgram->getPixelShader()->GetBufferPointer()),
            shaderProgram->getPixelShader()->GetBufferSize()
        };
    }

    psoDesc.RasterizerState       = rasterDesc;
    psoDesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState     = depthStencilDesc;
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.SampleDesc.Count   = 1;
    psoDesc.SampleDesc.Quality = 0;

    psoDesc.NumRenderTargets = rtvFormats.size();
    for (size_t i = 0; i < rtvFormats.size(); i++) {
        psoDesc.RTVFormats[i] = rtvFormats[i];
    }

    psoDesc.DSVFormat = dsvFormat;

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_Pso.GetAddressOf())));

    m_ShaderProgram = shaderProgram;
}

void DxRenderPass::bind(ID3D12GraphicsCommandList* commandList) {
    m_ShaderProgram->bind(commandList);
    commandList->SetPipelineState(m_Pso.Get());
}

}  // namespace Engine