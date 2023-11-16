#pragma once

#include "DxComputeProgram.hpp"

namespace Engine {

DxComputeProgram::DxComputeProgram(
    ID3D12Device* device, const std::string& file, const std::vector<ShaderProgramSlotDesc>& slots
) {
    m_Device = device;

    m_ComputeShader = DxUtils::CompileShader(file, nullptr, "CS", "cs_5_1");

    m_Slots = slots;
    m_Resources.resize(slots.size());

    //////////////////// Root Signature ////////////////////

    // Shader programs typically require resources as input (constant buffers,
    // textures, samplers). The root signature defines the resources the shader
    // programs expect. If we think of the shader programs as a function, and
    // the input resources as function parameters, then the root signature can be
    // thought of as defining the function signature.

    // Root signature is defined by an array of root parameters that describe the resources the shaders expect for a
    // draw call Root parameter can be a table, root descriptor or root constants.
    std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;
    slotRootParameters.resize(slots.size());

    std::vector<CD3DX12_DESCRIPTOR_RANGE> texTables;
    texTables.reserve(100);

    size_t uavNum = 0;

    for (size_t i = 0; i < slots.size(); i++) {
        slotRootParameters[i].InitAsConstantBufferView(i);

        auto slot = slots[i];

        if (slot.type == SHADER_PROGRAM_SLOT_TYPE::DATA) {
            slotRootParameters[i].InitAsConstantBufferView(i);
        } else if (slot.type == SHADER_PROGRAM_SLOT_TYPE::DATA_ARRAY) {
            texTables.emplace_back();
            slotRootParameters[i].InitAsShaderResourceView(texTables.size() - 1);
        } else if (slot.type == SHADER_PROGRAM_SLOT_TYPE::READ_WRITE_DATA) {
            slotRootParameters[i].InitAsUnorderedAccessView(uavNum++);
        } else if (slot.type == SHADER_PROGRAM_SLOT_TYPE::TEXTURE) {
            texTables.emplace_back();
            texTables[texTables.size() - 1].Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1,                     // Number of descriptors in table
                texTables.size() - 1,  // base shader register arguments are bound to for this root parameter
                0,                     // register space
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND  // offset from start of table
            );

            slotRootParameters[i].InitAsDescriptorTable(
                1, &texTables[texTables.size() - 1], D3D12_SHADER_VISIBILITY_PIXEL
            );
        } else if (slot.type == SHADER_PROGRAM_SLOT_TYPE::TEXTURE_ARRAY_4) {
            texTables.emplace_back();
            texTables[texTables.size() - 1].Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                4,                     // Number of descriptors in table
                texTables.size() - 1,  // base shader register arguments are bound to for this root parameter
                0,                     // register space
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND  // offset from start of table
            );

            slotRootParameters[i].InitAsDescriptorTable(
                1, &texTables[texTables.size() - 1], D3D12_SHADER_VISIBILITY_PIXEL
            );
        }
    }

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        slotRootParameters.size(), slotRootParameters.data(), 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    // create a root signature with a two slots. Each slot points to a descriptor range consisting of a constant buffer
    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob         = nullptr;
    HRESULT                          hr                = D3D12SerializeRootSignature(
        &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()
    );

    if (errorBlob != nullptr) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature(
        0,                                      // For single GPU operation, set this to zero
        serializedRootSig->GetBufferPointer(),  // A pointer to the source data for the serialized signature.
        serializedRootSig->GetBufferSize(
        ),  // The size, in bytes, of the block of memory that pBlobWithRootSignature points to.
        IID_PPV_ARGS(&m_RootSignature)  // The globally unique identifier (GUID) for the root signature interface.
    ));
}

void DxComputeProgram::setDataSlot(size_t index, std::shared_ptr<DxShaderProgramDataBuffer> buffer) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(buffer);
    m_CommandList->SetComputeRootConstantBufferView(index, buffer->resource()->GetGPUVirtualAddress());
}

void DxComputeProgram::setDataArraySlot(size_t index, std::shared_ptr<DxShaderProgramDataBuffer> buffer) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(buffer);
    m_CommandList->SetComputeRootShaderResourceView(index, buffer->resource()->GetGPUVirtualAddress());
}

void DxComputeProgram::setReadWriteDataSlot(size_t index, std::shared_ptr<DxReadWriteDataBuffer> buffer) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(buffer);
    m_CommandList->SetComputeRootUnorderedAccessView(index, buffer->resource()->GetGPUVirtualAddress());
}

void DxComputeProgram::setTextureSlot(size_t index, std::shared_ptr<DxTexture> texture) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(texture);
    m_CommandList->SetComputeRootDescriptorTable(index, texture->getSrvDescriptor().gpu);
}

void DxComputeProgram::setTextureSlot(size_t index, std::shared_ptr<DxRenderTexture> renderTexture) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(renderTexture);
    m_CommandList->SetComputeRootDescriptorTable(index, renderTexture->getSrvDescriptor().gpu);
}

void DxComputeProgram::setTextureSlot(size_t index, std::shared_ptr<DxDepthStencilTexture> dsTexture) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(dsTexture);
    m_CommandList->SetComputeRootDescriptorTable(index, dsTexture->getSrvDescriptor().gpu);
}

void DxComputeProgram::bind(ID3D12GraphicsCommandList* commandList) {
    m_CommandList = commandList;
    commandList->SetComputeRootSignature(m_RootSignature.Get());
}

}  // namespace Engine