#pragma once

#include "DxShaderProgram.hpp"

namespace Engine {

DxShaderProgram::DxShaderProgram(
    ID3D12Device* device,
    const std::string& vertexFile,
    const std::string& pixelFile,
    const std::vector<size_t>& dataSlots,
    size_t textureSlots) {   
    m_Device = device;

    m_VertexShader = DxUtils::CompileShader(vertexFile, nullptr, "VS", "vs_5_1");
    m_PixelShader = DxUtils::CompileShader(pixelFile, nullptr, "PS", "ps_5_1");

    //////////////////// Root Signature ////////////////////

    // Shader programs typically require resources as input (constant buffers,
    // textures, samplers). The root signature defines the resources the shader
    // programs expect. If we think of the shader programs as a function, and
    // the input resources as function parameters, then the root signature can be
    // thought of as defining the function signature.  

    // Root signature is defined by an array of root parameters that describe the resources the shaders expect for a draw call
    // Root parameter can be a table, root descriptor or root constants.
    std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;
    slotRootParameters.reserve(dataSlots.size() + textureSlots);

    for (size_t i = 0; i < dataSlots.size(); i++) {
        slotRootParameters[i].InitAsConstantBufferView(i);
        m_DataSlots.push_back(std::make_unique<DxShaderProgramSlot>(m_Device, dataSlots[i]));
    }

    if (textureSlots > 0) {
        CD3DX12_DESCRIPTOR_RANGE texTable;
        texTable.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            textureSlots, // Number of descriptors in table
            0,            // base shader register arguments are bound to for this root parameter
            0,            // register space
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND   // offset from start of table
        );

        slotRootParameters[dataSlots.size()].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    }

    auto staticSamplers = getStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        slotRootParameters.size(),
        slotRootParameters.data(),
        (UINT)staticSamplers.size(),
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    // create a root signature with a two slots. Each slot points to a descriptor range consisting of a constant buffer
    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(),
        errorBlob.GetAddressOf()
    );
    
    if (errorBlob != nullptr) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature(
        0,                                      // For single GPU operation, set this to zero
        serializedRootSig->GetBufferPointer(),  // A pointer to the source data for the serialized signature.
        serializedRootSig->GetBufferSize(),     // The size, in bytes, of the block of memory that pBlobWithRootSignature points to.
        IID_PPV_ARGS(&m_RootSignature)          // The globally unique identifier (GUID) for the root signature interface. 
    ));

    m_InputLayout =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void DxShaderProgram::setDataSlot(size_t index, void* data) {
    m_DataSlots[index]->copyData(data);
}

void DxShaderProgram::setTextureSlot(size_t index, D3D12_GPU_DESCRIPTOR_HANDLE srvDescriptor) {
    m_TxtureSlots[index] = srvDescriptor;
}

void DxShaderProgram::setTextureSlot(size_t index, std::shared_ptr<DxRenderTexture> renderTexture) {
    setTextureSlot(index, renderTexture->getSrvDescriptor().gpu);
}

void DxShaderProgram::bind(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());

    for (size_t i = 0; i < m_DataSlots.size(); i++) {
        commandList->SetGraphicsRootConstantBufferView(i, m_DataSlots[i]->resource()->GetGPUVirtualAddress());
    }
    
    for (size_t i = 0; i < m_TxtureSlots.size(); i++) {
        commandList->SetGraphicsRootDescriptorTable(i, m_TxtureSlots[i]);
    }
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> DxShaderProgram::getStaticSamplers() {
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.  

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
        0.0f,                              // mipLODBias
        8);                                // maxAnisotropy

    return { 
        pointWrap,
        pointClamp,
        linearWrap,
        linearClamp, 
        anisotropicWrap,
        anisotropicClamp
    };
}

} // namespace Engine