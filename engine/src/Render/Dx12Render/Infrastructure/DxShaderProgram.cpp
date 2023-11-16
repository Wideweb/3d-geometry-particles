#pragma once

#include "DxShaderProgram.hpp"

namespace Engine {

DxShaderProgram::DxShaderProgram(
    ID3D12Device* device, const std::string& vertexFile, const std::string& pixelFile,
    const std::vector<ShaderProgramSlotDesc>& slots
) {
    m_Device = device;

    m_VertexShader = DxUtils::CompileShader(vertexFile, nullptr, "VS", "vs_5_1");

    if (pixelFile != "") {
        m_PixelShader = DxUtils::CompileShader(pixelFile, nullptr, "PS", "ps_5_1");
    }

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

    for (size_t i = 0; i < slots.size(); i++) {
        slotRootParameters[i].InitAsConstantBufferView(i);

        auto slot = slots[i];

        if (slot.type == SHADER_PROGRAM_SLOT_TYPE::DATA) {
            slotRootParameters[i].InitAsConstantBufferView(i);
        } else if (slot.type == SHADER_PROGRAM_SLOT_TYPE::DATA_ARRAY) {
            texTables.emplace_back();
            slotRootParameters[i].InitAsShaderResourceView(i);
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

    auto staticSamplers = getStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        slotRootParameters.size(), slotRootParameters.data(), (UINT)staticSamplers.size(), staticSamplers.data(),
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

    m_InputLayout = {
        {"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
}

void DxShaderProgram::setDataSlot(size_t index, std::shared_ptr<DxShaderProgramDataBuffer> buffer) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(buffer);
    m_CommandList->SetGraphicsRootConstantBufferView(index, buffer->resource()->GetGPUVirtualAddress());
}

void DxShaderProgram::setDataArraySlot(size_t index, std::shared_ptr<DxShaderProgramDataBuffer> buffer) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(buffer);
    m_CommandList->SetGraphicsRootShaderResourceView(index, buffer->resource()->GetGPUVirtualAddress());
}

void DxShaderProgram::setTextureSlot(size_t index, std::shared_ptr<DxTexture> texture) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(texture);
    m_CommandList->SetGraphicsRootDescriptorTable(index, texture->getSrvDescriptor().gpu);
}

void DxShaderProgram::setTextureSlot(size_t index, std::shared_ptr<DxRenderTexture> renderTexture) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(renderTexture);
    m_CommandList->SetGraphicsRootDescriptorTable(index, renderTexture->getSrvDescriptor().gpu);
}

void DxShaderProgram::setTextureSlot(size_t index, std::shared_ptr<DxDepthStencilTexture> dsTexture) {
    m_Resources[index] = std::static_pointer_cast<DxResource>(dsTexture);
    m_CommandList->SetGraphicsRootDescriptorTable(index, dsTexture->getSrvDescriptor().gpu);
}

void DxShaderProgram::bind(ID3D12GraphicsCommandList* commandList) {
    m_CommandList = commandList;
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> DxShaderProgram::getStaticSamplers() {
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0,                                // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT,   // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP
    );  // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1,                                 // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT,    // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );  // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2,                                // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,  // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP
    );  // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3,                                 // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,   // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );  // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4,                                // shaderRegister
        D3D12_FILTER_ANISOTROPIC,         // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8
    );  // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5,                                 // shaderRegister
        D3D12_FILTER_ANISOTROPIC,          // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
        0.0f,                              // mipLODBias
        8
    );  // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC shadow(
        6,                                                 // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,  // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,                 // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,                 // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,                 // addressW
        0.0f,                                              // mipLODBias
        16,                                                // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK
    );

    return {pointWrap, pointClamp, linearWrap, linearClamp, anisotropicWrap, anisotropicClamp, shadow};
}

}  // namespace Engine