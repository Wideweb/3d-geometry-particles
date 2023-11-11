#include "DxTextureLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include "stb_image.hpp"

namespace Engine {

Microsoft::WRL::ComPtr<ID3D12Resource> DxTextureLoader::loadTextureFromFile(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& filename
) {
    // Загрузка изображения с помощью библиотеки stb_image
    int            width, height, channels;
    unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!imageData) {
        std::cout << "ERROR::TEXTURE::Failed to load texture from file: " << filename << std::endl;
        return nullptr;
    }

    // Создание ресурса текстуры на GPU
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

    CD3DX12_RESOURCE_DESC   textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);
    CD3DX12_HEAP_PROPERTIES textureHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = device->CreateCommittedResource(
        &textureHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(textureResource.GetAddressOf())
    );

    if (FAILED(hr)) {
        std::cout << "ERROR::TEXTURE::Failed to create texture resource." << std::endl;
        return nullptr;
    }

    // Копирование данных текстуры на GPU
    Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap = nullptr;

    CD3DX12_HEAP_PROPERTIES textureUploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC   textureUploadDesc = CD3DX12_RESOURCE_DESC::Buffer(width * height * 4);

    hr = device->CreateCommittedResource(
        &textureUploadHeapProperties, D3D12_HEAP_FLAG_NONE, &textureUploadDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(textureUploadHeap.GetAddressOf())
    );

    if (FAILED(hr)) {
        std::cout << "ERROR::TEXTURE::Failed to create texture upload heap." << std::endl;
        return nullptr;
    }

    D3D12_SUBRESOURCE_DATA textureDataDesc = {};
    textureDataDesc.pData                  = imageData;
    textureDataDesc.RowPitch               = width * 4;
    textureDataDesc.SlicePitch             = textureDataDesc.RowPitch * height;

    UpdateSubresources<1>(commandList, textureResource.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureDataDesc);

    // Установка состояния ресурса текстуры на GPU
    CD3DX12_RESOURCE_BARRIER textureBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );

    commandList->ResourceBarrier(1, &textureBarrier);

    return textureResource;
}

void DxTextureLoader::loadCubeMapDataFromFile(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::array<std::string, 6>& files,
    Microsoft::WRL::ComPtr<ID3D12Resource>& textureResource, Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap
) {
    HRESULT hr;

    D3D12_RESOURCE_DESC textureDesc;

    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    for (int i = 0; i < 6; i++) {
        BYTE* imageData;
        int   imageBytesPerRow;
        int   imageSize = DxTextureLoader::loadImageDataFromFile(
            &imageData, textureDesc, DxUtils::AnsiToWString(files[i]).c_str(), imageBytesPerRow
        );
        // make sure we have data
        if (imageSize <= 0) {
            std::cout << "Image has no size -ERROR:" << std::endl;
            return;
        }

        // store texture buffer in upload heap
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData                  = &imageData[0];              // pointer to our image data
        textureData.RowPitch               = imageBytesPerRow;           // size of all our triangle vertex data
        textureData.SlicePitch = imageBytesPerRow * textureDesc.Height;  // also the size of our triangle vertex data

        subresources.push_back(textureData);
    }

    textureResource.Reset();

    // create a default heap where the upload heap will copy its contents into (contents being the texture)
    CD3DX12_HEAP_PROPERTIES textureHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
    textureDesc.DepthOrArraySize = 6;

    hr = device->CreateCommittedResource(
        &textureHeapProperties,  // a default heap
        D3D12_HEAP_FLAG_NONE,    // no flags
        &textureDesc,            // the description of our texture
        D3D12_RESOURCE_STATE_COPY_DEST,
        // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
        nullptr,  // used for render targets and depth/stencil buffers
        IID_PPV_ARGS(textureResource.GetAddressOf())
    );
    if (FAILED(hr)) {
        std::cout << "Error create default heap in LoadTexture -ERROR:" + std::to_string(hr) << std::endl;
        return;
    }
    textureResource->SetName(L"Texture Buffer Resource Heap");

    UINT64 textureUploadBufferSize;
    // this function gets the size an upload buffer needs to be to upload a texture to the gpu.
    // each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
    // eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width *
    // numBytesPerPixel);
    // textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
    device->GetCopyableFootprints(&textureDesc, 0, 6, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

    // now we create an upload heap to upload our texture to the GPU
    textureUploadHeap.Reset();

    CD3DX12_HEAP_PROPERTIES textureUploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC   textureUploadDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);

    hr = device->CreateCommittedResource(
        &textureUploadHeapProperties,  // upload heap
        D3D12_HEAP_FLAG_NONE,          // no flags
        &textureUploadDesc,
        // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
        D3D12_RESOURCE_STATE_GENERIC_READ,  // We will copy the contents from this heap to the default heap above
        nullptr, IID_PPV_ARGS(textureUploadHeap.GetAddressOf())
    );
    if (FAILED(hr)) {
        std::cout << "Error create upload heap in LoadTexture -ERROR:" << std::to_string(hr) << std::endl;
        return;
    }
    textureUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

    // Now we copy the upload buffer contents to the default heap
    UpdateSubresources(commandList, textureResource.Get(), textureUploadHeap.Get(), 0, 0, 6, subresources.data());

    // transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel
    // shader to get the color of pixels)
    CD3DX12_RESOURCE_BARRIER textureBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );

    commandList->ResourceBarrier(1, &textureBarrier);
}

void DxTextureLoader::loadImageDataFromFile(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& filename,
    Microsoft::WRL::ComPtr<ID3D12Resource>& textureResource, Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap
) {
    HRESULT hr;
    // Load the image from file
    BYTE*               imageData;
    D3D12_RESOURCE_DESC textureDesc;
    int                 imageBytesPerRow;
    int                 imageSize = DxTextureLoader::loadImageDataFromFile(
        &imageData, textureDesc, DxUtils::AnsiToWString(filename).c_str(), imageBytesPerRow
    );
    // make sure we have data
    if (imageSize <= 0) {
        std::cout << "Image has no size -ERROR:" << std::endl;
        return;
    }

    textureResource.Reset();

    // create a default heap where the upload heap will copy its contents into (contents being the texture)
    CD3DX12_HEAP_PROPERTIES textureHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

    hr = device->CreateCommittedResource(
        &textureHeapProperties,  // a default heap
        D3D12_HEAP_FLAG_NONE,    // no flags
        &textureDesc,            // the description of our texture
        D3D12_RESOURCE_STATE_COPY_DEST,
        // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
        nullptr,  // used for render targets and depth/stencil buffers
        IID_PPV_ARGS(textureResource.GetAddressOf())
    );
    if (FAILED(hr)) {
        std::cout << "Error create default heap in LoadTexture -ERROR:" + std::to_string(hr) << std::endl;
        return;
    }
    textureResource->SetName(L"Texture Buffer Resource Heap");

    UINT64 textureUploadBufferSize;
    // this function gets the size an upload buffer needs to be to upload a texture to the gpu.
    // each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
    // eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width *
    // numBytesPerPixel);
    // textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
    device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

    // now we create an upload heap to upload our texture to the GPU
    textureUploadHeap.Reset();

    CD3DX12_HEAP_PROPERTIES textureUploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC   textureUploadDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);

    hr = device->CreateCommittedResource(
        &textureUploadHeapProperties,  // upload heap
        D3D12_HEAP_FLAG_NONE,          // no flags
        &textureUploadDesc,
        // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
        D3D12_RESOURCE_STATE_GENERIC_READ,  // We will copy the contents from this heap to the default heap above
        nullptr, IID_PPV_ARGS(textureUploadHeap.GetAddressOf())
    );
    if (FAILED(hr)) {
        std::cout << "Error create upload heap in LoadTexture -ERROR:" << std::to_string(hr) << std::endl;
        return;
    }
    textureUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

    // store texture buffer in upload heap
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData                  = &imageData[0];              // pointer to our image data
    textureData.RowPitch               = imageBytesPerRow;           // size of all our triangle vertex data
    textureData.SlicePitch = imageBytesPerRow * textureDesc.Height;  // also the size of our triangle vertex data

    // Now we copy the upload buffer contents to the default heap
    UpdateSubresources(commandList, textureResource.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);

    // transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel
    // shader to get the color of pixels)
    CD3DX12_RESOURCE_BARRIER textureBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );

    commandList->ResourceBarrier(1, &textureBarrier);
}

// load and decode image from file
int DxTextureLoader::loadImageDataFromFile(
    BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow
) {
    HRESULT hr;

    // we only need one instance of the imaging factory to create decoders and frames
    static IWICImagingFactory* wicFactory;

    // reset decoder, frame and converter since these will be different for each image we load
    IWICBitmapDecoder*     wicDecoder   = nullptr;
    IWICBitmapFrameDecode* wicFrame     = nullptr;
    IWICFormatConverter*   wicConverter = nullptr;

    bool imageConverted = false;

    if (wicFactory == nullptr) {
        // Initialize the COM library
        CoInitialize(nullptr);

        // create the WIC factory
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
        if (FAILED(hr))
            return 0;
    }

    // load a decoder for the image
    hr = wicFactory->CreateDecoderFromFilename(
        filename,                      // Image we want to load in
        nullptr,                       // This is a vendor ID, we do not prefer a specific one so set to null
        GENERIC_READ,                  // We want to read from this file
        WICDecodeMetadataCacheOnLoad,  // We will cache the metadata right away, rather than when needed, which might be
                                       // unknown
        &wicDecoder                    // the wic decoder to be created
    );
    if (FAILED(hr))
        return 0;

    // get image from decoder (this will decode the "frame")
    hr = wicDecoder->GetFrame(0, &wicFrame);
    if (FAILED(hr))
        return 0;

    // get wic pixel format of image
    WICPixelFormatGUID pixelFormat;
    hr = wicFrame->GetPixelFormat(&pixelFormat);
    if (FAILED(hr))
        return 0;

    // get size of image
    UINT textureWidth, textureHeight;
    hr = wicFrame->GetSize(&textureWidth, &textureHeight);
    if (FAILED(hr))
        return 0;

    // we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
    // out how to implement the support yourself

    // convert wic pixel format to dxgi pixel format
    DXGI_FORMAT dxgiFormat = getDXGIFormatFromWICFormat(pixelFormat);

    // if the format of the image is not a supported dxgi format, try to convert it
    if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
        // get a dxgi compatible wic format from the current image format
        WICPixelFormatGUID convertToPixelFormat = getConvertToWICFormat(pixelFormat);

        // return if no dxgi compatible format was found
        if (convertToPixelFormat == GUID_WICPixelFormatDontCare)
            return 0;

        // set the dxgi format
        dxgiFormat = getDXGIFormatFromWICFormat(convertToPixelFormat);

        // create the format converter
        hr = wicFactory->CreateFormatConverter(&wicConverter);
        if (FAILED(hr))
            return 0;

        // make sure we can convert to the dxgi compatible format
        BOOL canConvert = FALSE;
        hr              = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
        if (FAILED(hr) || !canConvert)
            return 0;

        // do the conversion (wicConverter will contain the converted image)
        hr = wicConverter->Initialize(
            wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom
        );
        if (FAILED(hr))
            return 0;

        // this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
        imageConverted = true;
    }

    int bitsPerPixel = getDXGIFormatBitsPerPixel(dxgiFormat);  // number of bits per pixel
    bytesPerRow      = (textureWidth * bitsPerPixel) / 8;      // number of bytes in each row of the image data
    int imageSize    = bytesPerRow * textureHeight;            // total image size in bytes

    // allocate enough memory for the raw image data, and set imageData to point to that memory
    *imageData = (BYTE*)malloc(imageSize);

    // copy (decoded) raw image data into the newly allocated memory (imageData)
    if (imageConverted) {
        // if image format needed to be converted, the wic converter will contain the converted image
        hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
        if (FAILED(hr))
            return 0;
    } else {
        // no need to convert, just copy data from the wic frame
        hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
        if (FAILED(hr))
            return 0;
    }

    // now describe the texture with the information we have obtained from the image
    resourceDescription           = {};
    resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDescription.Alignment = 0;  // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB
                                        // (4MB for multi-sampled textures)
    resourceDescription.Width            = textureWidth;   // width of the texture
    resourceDescription.Height           = textureHeight;  // height of the texture
    resourceDescription.DepthOrArraySize = 1;  // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D
                                               // textures (we only have one image, so we set 1)
    resourceDescription.MipLevels =
        1;  // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
    resourceDescription.Format           = dxgiFormat;  // This is the dxgi format of the image (format of the pixels)
    resourceDescription.SampleDesc.Count = 1;  // This is the number of samples per pixel, we just want 1 sample
    resourceDescription.SampleDesc.Quality =
        0;  // The quality level of the samples. Higher is better quality, but worse performance
    resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // The arrangement of the pixels. Setting to unknown
                                                                // lets the driver choose the most efficient one
    resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;       // no flags

    // return the size of the image. remember to delete the image once your done with it (in this tutorial once its
    // uploaded to the gpu)
    return imageSize;
}

DXGI_FORMAT DxTextureLoader::getDXGIFormatFromWICFormat(WICPixelFormatGUID& wic_format_guid) {
    if (wic_format_guid == GUID_WICPixelFormat128bppRGBAFloat)
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    if (wic_format_guid == GUID_WICPixelFormat64bppRGBAHalf)
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    if (wic_format_guid == GUID_WICPixelFormat64bppRGBA)
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat32bppRGBA)
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat32bppBGRA)
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat32bppBGR)
        return DXGI_FORMAT_B8G8R8X8_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat32bppRGBA1010102XR)
        return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat32bppRGBA1010102)
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat16bppBGRA5551)
        return DXGI_FORMAT_B5G5R5A1_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat16bppBGR565)
        return DXGI_FORMAT_B5G6R5_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat32bppGrayFloat)
        return DXGI_FORMAT_R32_FLOAT;
    if (wic_format_guid == GUID_WICPixelFormat16bppGrayHalf)
        return DXGI_FORMAT_R16_FLOAT;
    if (wic_format_guid == GUID_WICPixelFormat16bppGray)
        return DXGI_FORMAT_R16_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat8bppGray)
        return DXGI_FORMAT_R8_UNORM;
    if (wic_format_guid == GUID_WICPixelFormat8bppAlpha)
        return DXGI_FORMAT_A8_UNORM;
    return DXGI_FORMAT_UNKNOWN;
}

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID DxTextureLoader::getConvertToWICFormat(WICPixelFormatGUID& wic_format_guid) {
    if (wic_format_guid == GUID_WICPixelFormatBlackWhite)
        return GUID_WICPixelFormat8bppGray;
    if (wic_format_guid == GUID_WICPixelFormat1bppIndexed)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat2bppIndexed)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat4bppIndexed)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat8bppIndexed)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat2bppGray)
        return GUID_WICPixelFormat8bppGray;
    if (wic_format_guid == GUID_WICPixelFormat4bppGray)
        return GUID_WICPixelFormat8bppGray;
    if (wic_format_guid == GUID_WICPixelFormat16bppGrayFixedPoint)
        return GUID_WICPixelFormat16bppGrayHalf;
    if (wic_format_guid == GUID_WICPixelFormat32bppGrayFixedPoint)
        return GUID_WICPixelFormat32bppGrayFloat;
    if (wic_format_guid == GUID_WICPixelFormat16bppBGR555)
        return GUID_WICPixelFormat16bppBGRA5551;
    if (wic_format_guid == GUID_WICPixelFormat32bppBGR101010)
        return GUID_WICPixelFormat32bppRGBA1010102;
    if (wic_format_guid == GUID_WICPixelFormat24bppBGR)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat24bppRGB)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat32bppPBGRA)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat32bppPRGBA)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat48bppRGB)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat48bppBGR)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat64bppBGRA)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat64bppPRGBA)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat64bppPBGRA)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat48bppRGBFixedPoint)
        return GUID_WICPixelFormat64bppRGBAHalf;
    if (wic_format_guid == GUID_WICPixelFormat48bppBGRFixedPoint)
        return GUID_WICPixelFormat64bppRGBAHalf;
    if (wic_format_guid == GUID_WICPixelFormat64bppRGBAFixedPoint)
        return GUID_WICPixelFormat64bppRGBAHalf;
    if (wic_format_guid == GUID_WICPixelFormat64bppBGRAFixedPoint)
        return GUID_WICPixelFormat64bppRGBAHalf;
    if (wic_format_guid == GUID_WICPixelFormat64bppRGBFixedPoint)
        return GUID_WICPixelFormat64bppRGBAHalf;
    if (wic_format_guid == GUID_WICPixelFormat64bppRGBHalf)
        return GUID_WICPixelFormat64bppRGBAHalf;
    if (wic_format_guid == GUID_WICPixelFormat48bppRGBHalf)
        return GUID_WICPixelFormat64bppRGBAHalf;
    if (wic_format_guid == GUID_WICPixelFormat128bppPRGBAFloat)
        return GUID_WICPixelFormat128bppRGBAFloat;
    if (wic_format_guid == GUID_WICPixelFormat128bppRGBFloat)
        return GUID_WICPixelFormat128bppRGBAFloat;
    if (wic_format_guid == GUID_WICPixelFormat128bppRGBAFixedPoint)
        return GUID_WICPixelFormat128bppRGBAFloat;
    if (wic_format_guid == GUID_WICPixelFormat128bppRGBFixedPoint)
        return GUID_WICPixelFormat128bppRGBAFloat;
    if (wic_format_guid == GUID_WICPixelFormat32bppRGBE)
        return GUID_WICPixelFormat128bppRGBAFloat;
    if (wic_format_guid == GUID_WICPixelFormat32bppCMYK)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat64bppCMYK)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat40bppCMYKAlpha)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat80bppCMYKAlpha)
        return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
    if (wic_format_guid == GUID_WICPixelFormat32bppRGB)
        return GUID_WICPixelFormat32bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat64bppRGB)
        return GUID_WICPixelFormat64bppRGBA;
    if (wic_format_guid == GUID_WICPixelFormat64bppPRGBAHalf)
        return GUID_WICPixelFormat64bppRGBAHalf;
#endif
    return GUID_WICPixelFormatDontCare;
}

// get the number of bits per pixel for a dxgi format
int DxTextureLoader::getDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgi_format) {
    if (dxgi_format == DXGI_FORMAT_R32G32B32A32_FLOAT)
        return 128;
    if (dxgi_format == DXGI_FORMAT_R16G16B16A16_FLOAT)
        return 64;
    if (dxgi_format == DXGI_FORMAT_R16G16B16A16_UNORM)
        return 64;
    if (dxgi_format == DXGI_FORMAT_R8G8B8A8_UNORM)
        return 32;
    if (dxgi_format == DXGI_FORMAT_B8G8R8A8_UNORM)
        return 32;
    if (dxgi_format == DXGI_FORMAT_B8G8R8X8_UNORM)
        return 32;
    if (dxgi_format == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM)
        return 32;
    if (dxgi_format == DXGI_FORMAT_R10G10B10A2_UNORM)
        return 32;
    if (dxgi_format == DXGI_FORMAT_B5G5R5A1_UNORM)
        return 16;
    if (dxgi_format == DXGI_FORMAT_B5G6R5_UNORM)
        return 16;
    else {
        if (dxgi_format == DXGI_FORMAT_R32_FLOAT)
            return 32;
        if (dxgi_format == DXGI_FORMAT_R16_FLOAT)
            return 16;
        if (dxgi_format == DXGI_FORMAT_R16_UNORM)
            return 16;
        if (dxgi_format == DXGI_FORMAT_R8_UNORM)
            return 8;
        if (dxgi_format == DXGI_FORMAT_A8_UNORM)
            return 8;
    }
}

}  // namespace Engine