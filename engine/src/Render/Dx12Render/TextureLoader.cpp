#include "TextureLoader.hpp"

#include "stb_image.hpp"

ID3D12Resource* TextureLoader::LoadTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& filename) {
    // Загрузка изображения с помощью библиотеки stb_image
    int width, height, channels;
    unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!imageData) {
        std::cout << "ERROR::TEXTURE::Failed to load texture from file: " << filename << std::endl;
        return nullptr;
    }

    // Создание ресурса текстуры на GPU
    ID3D12Resource* textureResource = nullptr;

    CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);
    CD3DX12_HEAP_PROPERTIES textureHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = device->CreateCommittedResource(
        &textureHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&textureResource)
    );

    if (FAILED(hr)) {
        std::cout << "ERROR::TEXTURE::Failed to create texture resource." << std::endl;
        return nullptr;
    }

    // Копирование данных текстуры на GPU
    ID3D12Resource* textureUploadHeap = nullptr;

    CD3DX12_HEAP_PROPERTIES textureUploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC textureUploadDesc = CD3DX12_RESOURCE_DESC::Buffer(width * height * 4);

    hr = device->CreateCommittedResource(
        &textureUploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &textureUploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap)
    );

    if (FAILED(hr)) {
        std::cout << "ERROR::TEXTURE::Failed to create texture upload heap." << std::endl;
        return nullptr;
    }

    D3D12_SUBRESOURCE_DATA textureDataDesc = {};
    textureDataDesc.pData = imageData;
    textureDataDesc.RowPitch = width * 4;
    textureDataDesc.SlicePitch = textureDataDesc.RowPitch * height;

    UpdateSubresources<1>(commandList, textureResource, textureUploadHeap, 0, 0, 1, &textureDataDesc);

    // Установка состояния ресурса текстуры на GPU
    CD3DX12_RESOURCE_BARRIER textureBarrier = CD3DX12_RESOURCE_BARRIER::Transition(textureResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    commandList->ResourceBarrier(1, &textureBarrier);

    return textureResource;
}