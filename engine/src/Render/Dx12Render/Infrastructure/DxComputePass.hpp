#pragma once

#include <memory>
#include <vector>

#include "DxComputeProgram.hpp"
#include "DxUtils.hpp"

namespace Engine {

class DxComputePass {
public:
    DxComputePass(ID3D12Device* device, std::shared_ptr<DxComputeProgram> computeProgram);

    void bind(ID3D12GraphicsCommandList* commandList);

    ID3D12PipelineState* resource() { return m_Pso.Get(); }

private:
    std::shared_ptr<DxComputeProgram>           m_ComputeProgram;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_Pso;
};

}  // namespace Engine