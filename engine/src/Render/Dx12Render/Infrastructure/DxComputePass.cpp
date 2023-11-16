#include "DxComputePass.hpp"

namespace Engine {

DxComputePass::DxComputePass(ID3D12Device* device, std::shared_ptr<DxComputeProgram> computeProgram) {
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature                    = computeProgram->getRootSignature();
    psoDesc.CS                                = {
        reinterpret_cast<BYTE*>(computeProgram->getComputeShader()->GetBufferPointer()),
        computeProgram->getComputeShader()->GetBufferSize()
    };
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_Pso.GetAddressOf())));

    m_ComputeProgram = computeProgram;
}

void DxComputePass::bind(ID3D12GraphicsCommandList* commandList) {
    m_ComputeProgram->bind(commandList);
    commandList->SetPipelineState(m_Pso.Get());
}

}  // namespace Engine