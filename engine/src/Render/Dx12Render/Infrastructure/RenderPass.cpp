#include "RenderPass.hpp"

RenderPass::RenderPass(
    ID3D12Device* device,
    std::shared_ptr<ShaderProgram> shaderProgram,
    size_t constantBuffersNum,
    size_t texturesNum,
    std::vector<DXGI_FORMAT> rtvFormats,
    DXGI_FORMAT dsvFormat
) {
    //////////////////// Root Signature ////////////////////

    // Shader programs typically require resources as input (constant buffers,
    // textures, samplers). The root signature defines the resources the shader
    // programs expect. If we think of the shader programs as a function, and
    // the input resources as function parameters, then the root signature can be
    // thought of as defining the function signature.  

    // Root signature is defined by an array of root parameters that describe the resources the shaders expect for a draw call
    // Root parameter can be a table, root descriptor or root constants.
    size_t slotsNum = constantBuffersNum + texturesNum;
    std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;
    slotRootParameters.reserve(slotsNum);

    for (size_t i = 0; i < constantBuffersNum; i++) {
        slotRootParameters[i].InitAsConstantBufferView(i);
    }

    if (texturesNum > 0) {
        CD3DX12_DESCRIPTOR_RANGE texTable;
        texTable.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            texturesNum,  // Number of descriptors in table
            0,            // base shader register arguments are bound to for this root parameter
            0,            // register space
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND   // offset from start of table
        );

        slotRootParameters[constantBuffersNum].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    }

    auto staticSamplers = getStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        slotsNum,
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
        IID_PPV_ARGS(&m_RootSignature)            // The globally unique identifier (GUID) for the root signature interface. 
    ));
    
    //////////////////// Pipeline State Objects ////////////////////
    CD3DX12_RASTERIZER_DESC rasterDesc(D3D12_DEFAULT);
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterDesc.FrontCounterClockwise = true;
    rasterDesc.DepthClipEnable = true;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { shaderProgram->getInputLayout().data(), (UINT)shaderProgram->getInputLayout().size() };
    psoDesc.pRootSignature = m_RootSignature.Get();
    psoDesc.VS = { 
        reinterpret_cast<BYTE*>(shaderProgram->getVertexShader()->GetBufferPointer()),
        shaderProgram->getVertexShader()->GetBufferSize()
    };
    psoDesc.PS = { 
        reinterpret_cast<BYTE*>(shaderProgram->getPixelShader()->GetBufferPointer()),
        shaderProgram->getPixelShader()->GetBufferSize()
    };
    psoDesc.RasterizerState = rasterDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    psoDesc.NumRenderTargets = rtvFormats.size();
    for (size_t i = 0; i < rtvFormats.size(); i++) {
        psoDesc.RTVFormats[i] = rtvFormats[i];
    }

    psoDesc.DSVFormat = dsvFormat;
    
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_Pso.GetAddressOf())));
}

void RenderPass::bind(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());
    commandList->SetPipelineState(m_Pso.Get());
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> RenderPass::getStaticSamplers() {
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