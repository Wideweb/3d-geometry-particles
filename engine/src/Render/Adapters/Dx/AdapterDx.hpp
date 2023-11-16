#pragma once

#include "CrossPlatformRender.hpp"
#include "DxComputePass.hpp"
#include "DxComputeProgram.hpp"
#include "DxDepthStencilTexture.hpp"
#include "DxFramebuffer.hpp"
#include "DxReadWriteDataBuffer.hpp"
#include "DxRender.hpp"
#include "DxRenderPass.hpp"
#include "DxRenderTexture.hpp"
#include "DxShaderProgram.hpp"
#include "DxShaderProgramDataBuffer.hpp"
#include "DxTexture.hpp"

namespace Engine {

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// CULL FUNC /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
D3D12_CULL_MODE getDxCullMode(CULL_MODE mode) {
    switch (mode) {
        case CULL_MODE::NONE:
            return D3D12_CULL_MODE_NONE;
        case CULL_MODE::BACK:
            return D3D12_CULL_MODE_BACK;
        case CULL_MODE::FRONT:
            return D3D12_CULL_MODE_FRONT;
        default:
            throw std::invalid_argument("getDxCullMode: invalid cull mode.");
    }
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DEPTH FUNC ////////////////////////////////
////////////////////////////////////////////////////////////////////////////
D3D12_COMPARISON_FUNC getDxDepthFunc(DEPTH_FUNC func) {
    switch (func) {
        case DEPTH_FUNC::NEVER:
            return D3D12_COMPARISON_FUNC_NEVER;
        case DEPTH_FUNC::LESS:
            return D3D12_COMPARISON_FUNC_LESS;
        case DEPTH_FUNC::EQUAL:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case DEPTH_FUNC::LESS_EQUAL:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case DEPTH_FUNC::GREATER:
            return D3D12_COMPARISON_FUNC_GREATER;
        case DEPTH_FUNC::NOT_EQUAL:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case DEPTH_FUNC::GREATER_EQUAL:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case DEPTH_FUNC::ALWAYS:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            throw std::invalid_argument("getDxDepthFunc: invalid depth func.");
    }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////// TEXTURE FORMAT //////////////////////////////
////////////////////////////////////////////////////////////////////////////
DXGI_FORMAT getDxTextureFormat(CROSS_PLATFROM_TEXTURE_FORMATS format) {
    switch (format) {
        case CROSS_PLATFROM_TEXTURE_FORMATS::RGBA8:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case CROSS_PLATFROM_TEXTURE_FORMATS::D24S8:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        default:
            throw std::invalid_argument("getDxTextureFormat: invalid internal format.");
    }
}

////////////////////////////////////////////////////////////////////////////
///////////////////////////////// TEXTURE //////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxTextureWrapper : public CrossPlatformTexture {
public:
    DxTextureWrapper(std::shared_ptr<DxTexture> nativeTexture) { m_NativeTexture = nativeTexture; }

    std::shared_ptr<DxTexture> getNative() { return m_NativeTexture; }

private:
    std::shared_ptr<DxTexture> m_NativeTexture;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////// RENDER TEXTURE //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxRenderTextureWrapper : public CrossPlatformRenderTexture {
public:
    DxRenderTextureWrapper(std::shared_ptr<DxRenderTexture> nativeRT) { m_NativeRT = nativeRT; }

    void resize(size_t width, size_t height) override { m_NativeRT->resize(width, height); }

    std::shared_ptr<DxRenderTexture> getNative() { return m_NativeRT; }

private:
    std::shared_ptr<DxRenderTexture> m_NativeRT;
};

////////////////////////////////////////////////////////////////////////////
/////////////////////////// DEPTH STENCIL TEXTURE //////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxDepthStencilTextureWrapper : public CrossPlatformDepthStencilTexture {
public:
    DxDepthStencilTextureWrapper(std::shared_ptr<DxDepthStencilTexture> nativeDS) { m_NativeDS = nativeDS; }

    void resize(size_t width, size_t height) override { m_NativeDS->resize(width, height); }

    std::shared_ptr<DxDepthStencilTexture> getNative() { return m_NativeDS; }

private:
    std::shared_ptr<DxDepthStencilTexture> m_NativeDS;
};

////////////////////////////////////////////////////////////////////////////
/////////////////////// SHADER PROGRAM DATA BUFFER /////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxShaderProgramDataBufferWrapper : public CrossPlatformShaderProgramDataBuffer {
public:
    DxShaderProgramDataBufferWrapper(std::shared_ptr<DxShaderProgramDataBuffer> nativeBuffer) {
        m_NativeBuffer = nativeBuffer;
    }

    void copyData(void* data) override { m_NativeBuffer->copyData(data); }

    std::shared_ptr<DxShaderProgramDataBuffer> getNative() { return m_NativeBuffer; }

private:
    std::shared_ptr<DxShaderProgramDataBuffer> m_NativeBuffer;
};

////////////////////////////////////////////////////////////////////////////
/////////////////////// READ WRITE DATA BUFFER /////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxReadWriteDataBufferWrapper : public CrossPlatformReadWriteDataBuffer {
public:
    DxReadWriteDataBufferWrapper(std::shared_ptr<DxReadWriteDataBuffer> nativeBuffer) { m_NativeBuffer = nativeBuffer; }

    void  copyData(void* data) override { m_NativeBuffer->copyData(data); }
    void  copyData(void* data, size_t byteSize) override { m_NativeBuffer->copyData(data, byteSize); }
    void  readFromVRAM() override { m_NativeBuffer->readFromVRAM(); }
    void  writeToVRAM() override { m_NativeBuffer->writeToVRAM(); }
    void* data() override { return m_NativeBuffer->data(); }

    std::shared_ptr<DxReadWriteDataBuffer> getNative() { return m_NativeBuffer; }

private:
    std::shared_ptr<DxReadWriteDataBuffer> m_NativeBuffer;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////// SHADER PROGRAM //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxShaderProgramWrapper : public CrossPlatformShaderProgram {
public:
    DxShaderProgramWrapper(std::shared_ptr<DxShaderProgram> nativeSP) noexcept { m_NativeSP = nativeSP; }

    void setDataSlot(size_t index, std::shared_ptr<CrossPlatformShaderProgramDataBuffer> buffer) override {
        auto bufferWrapper = std::static_pointer_cast<DxShaderProgramDataBufferWrapper>(buffer);
        m_NativeSP->setDataSlot(index, bufferWrapper->getNative());
    }

    void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformTexture> texture) override {
        auto textureWrapper = std::static_pointer_cast<DxTextureWrapper>(texture);
        m_NativeSP->setTextureSlot(index, textureWrapper->getNative());
    }

    void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformRenderTexture> texture) override {
        auto textureWrapper = std::static_pointer_cast<DxRenderTextureWrapper>(texture);
        m_NativeSP->setTextureSlot(index, textureWrapper->getNative());
    }

    void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformDepthStencilTexture> texture) override {
        auto textureWrapper = std::static_pointer_cast<DxDepthStencilTextureWrapper>(texture);
        m_NativeSP->setTextureSlot(index, textureWrapper->getNative());
    }

    std::shared_ptr<DxShaderProgram> getNative() { return m_NativeSP; }

private:
    std::shared_ptr<DxShaderProgram> m_NativeSP;
};

////////////////////////////////////////////////////////////////////////////
///////////////////////////// COMPUTE PROGRAM //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxComputeProgramWrapper : public CrossPlatformComputeProgram {
public:
    DxComputeProgramWrapper(std::shared_ptr<DxComputeProgram> nativeCP) noexcept { m_NativeCP = nativeCP; }

    void setDataSlot(size_t index, std::shared_ptr<CrossPlatformShaderProgramDataBuffer> buffer) override {
        auto bufferWrapper = std::static_pointer_cast<DxShaderProgramDataBufferWrapper>(buffer);
        m_NativeCP->setDataSlot(index, bufferWrapper->getNative());
    }

    void setDataArraySlot(size_t index, std::shared_ptr<CrossPlatformShaderProgramDataBuffer> buffer) {
        auto bufferWrapper = std::static_pointer_cast<DxShaderProgramDataBufferWrapper>(buffer);
        m_NativeCP->setDataArraySlot(index, bufferWrapper->getNative());
    }

    void setReadWriteDataSlot(size_t index, std::shared_ptr<CrossPlatformReadWriteDataBuffer> buffer) {
        auto bufferWrapper = std::static_pointer_cast<DxReadWriteDataBufferWrapper>(buffer);
        m_NativeCP->setReadWriteDataSlot(index, bufferWrapper->getNative());
    }

    void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformTexture> texture) override {
        auto textureWrapper = std::static_pointer_cast<DxTextureWrapper>(texture);
        m_NativeCP->setTextureSlot(index, textureWrapper->getNative());
    }

    void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformRenderTexture> texture) override {
        auto textureWrapper = std::static_pointer_cast<DxRenderTextureWrapper>(texture);
        m_NativeCP->setTextureSlot(index, textureWrapper->getNative());
    }

    void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformDepthStencilTexture> texture) override {
        auto textureWrapper = std::static_pointer_cast<DxDepthStencilTextureWrapper>(texture);
        m_NativeCP->setTextureSlot(index, textureWrapper->getNative());
    }

    std::shared_ptr<DxComputeProgram> getNative() { return m_NativeCP; }

private:
    std::shared_ptr<DxComputeProgram> m_NativeCP;
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// RENDER PASS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxRenderPassWrapper : public CrossPlatformRenderPass {
public:
    DxRenderPassWrapper(std::shared_ptr<DxRenderPass> nativeRP) noexcept { m_NativeRP = nativeRP; }

    std::shared_ptr<DxRenderPass> getNative() { return m_NativeRP; }

private:
    std::shared_ptr<DxRenderPass> m_NativeRP;
};

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// COMPUTE PASS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxComputePassWrapper : public CrossPlatformComputePass {
public:
    DxComputePassWrapper(std::shared_ptr<DxComputePass> nativeCP) noexcept { m_NativeCP = nativeCP; }

    std::shared_ptr<DxComputePass> getNative() { return m_NativeCP; }

private:
    std::shared_ptr<DxComputePass> m_NativeCP;
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FRAMEBUFFER ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxFramebufferWrapper : public CrossPlatformFramebuffer {
public:
    DxFramebufferWrapper(std::shared_ptr<DxFramebuffer> nativeFB) noexcept { m_NativeFB = nativeFB; }

    void addAttachment(std::shared_ptr<CrossPlatformRenderTexture> attachment) override {
        auto textureWrapper = std::static_pointer_cast<DxRenderTextureWrapper>(attachment);
        m_NativeFB->addAttachment(textureWrapper->getNative());

        m_Attachments.push_back(attachment);
    }

    void setDSAttachment(std::shared_ptr<CrossPlatformDepthStencilTexture> attachment) override {
        auto dsTextureWrapper = std::static_pointer_cast<DxDepthStencilTextureWrapper>(attachment);
        m_NativeFB->setDSAttachment(dsTextureWrapper->getNative());

        m_DSAttachment = attachment;
    }

    void resize(size_t width, size_t height) override { m_NativeFB->resize(width, height); }

    const std::vector<std::shared_ptr<CrossPlatformRenderTexture>>& getAttachments() const noexcept override {
        return m_Attachments;
    }

    std::shared_ptr<CrossPlatformDepthStencilTexture> getDSAttachment() const noexcept override {
        return m_DSAttachment;
    }

    std::shared_ptr<DxFramebuffer> getNative() { return m_NativeFB; }

private:
    std::shared_ptr<DxFramebuffer>                           m_NativeFB;
    std::vector<std::shared_ptr<CrossPlatformRenderTexture>> m_Attachments;
    std::shared_ptr<CrossPlatformDepthStencilTexture>        m_DSAttachment;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////// RENDER //////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class DxPlatformRenderWrapper : public CrossPlatformRender {
public:
    DxPlatformRenderWrapper(void* window, uint32_t width, uint32_t height) {
        m_NativeRender = std::make_shared<DxRender>(window, width, height);
    }

    virtual ~DxPlatformRenderWrapper() = default;

    void beginInitialization() override { m_NativeRender->beginInitialization(); }

    void endInitialization() override { m_NativeRender->endInitialization(); }

    void beginFrame() override { m_NativeRender->beginFrame(); }

    void endFrame() override { m_NativeRender->endFrame(); }

    void resize(uint32_t width, uint32_t height) override { m_NativeRender->resize(width, height); }

    void clear(float r, float g, float b, float a) override { m_NativeRender->clear(r, g, b, a); }

    void setViewport(uint32_t width, uint32_t height) override { m_NativeRender->setViewport(width, height); }

    void setPass(std::shared_ptr<CrossPlatformRenderPass> pass) override {
        auto passWrapper = std::static_pointer_cast<DxRenderPassWrapper>(pass);
        m_NativeRender->setRenderPass(passWrapper->getNative());
    }

    void setComputePass(std::shared_ptr<CrossPlatformComputePass> pass) override {
        auto passWrapper = std::static_pointer_cast<DxComputePassWrapper>(pass);
        m_NativeRender->setComputePass(passWrapper->getNative());
    }

    void compute(size_t x, size_t y, size_t z) override { m_NativeRender->compute(x, y, z); }

    void setFramebuffer(std::shared_ptr<CrossPlatformFramebuffer> fb) override {
        if (fb == nullptr) {
            m_NativeRender->setFramebuffer(nullptr);
        } else {
            auto fbWrapper = std::static_pointer_cast<DxFramebufferWrapper>(fb);
            m_NativeRender->setFramebuffer(fbWrapper->getNative());
        }
    }

    void registerGeometry(
        const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
    ) override {
        m_NativeRender->registerGeometry(geometry, subGeometries, subMeshes);
    }

    std::shared_ptr<CrossPlatformTexture> loadTexture(const std::string& filename) override {
        auto nativeTexture = m_NativeRender->loadTexture(filename);
        return std::make_shared<DxTextureWrapper>(nativeTexture);
    }

    std::shared_ptr<CrossPlatformTexture> loadCubeTexture(const std::array<std::string, 6>& files) override {
        auto nativeTexture = m_NativeRender->loadCubeTexture(files);
        return std::make_shared<DxTextureWrapper>(nativeTexture);
    }

    std::shared_ptr<CrossPlatformDepthStencilTexture> createDepthStencilTexture(size_t width, size_t height) override {
        auto nativeDSTexture = m_NativeRender->createDepthStencilTexture(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height);
        return std::make_shared<DxDepthStencilTextureWrapper>(nativeDSTexture);
    }

    std::shared_ptr<CrossPlatformRenderTexture> createRenderTexture(
        CROSS_PLATFROM_TEXTURE_FORMATS format, size_t width, size_t height
    ) override {
        auto nativeRT =
            m_NativeRender->createRenderTexture(getDxTextureFormat(format), D3D12_RESOURCE_FLAG_NONE, width, height);
        return std::make_shared<DxRenderTextureWrapper>(nativeRT);
    }

    std::shared_ptr<CrossPlatformShaderProgram> createShaderProgram(
        const std::string& vertexFile, const std::string& pixelFile, const std::vector<ShaderProgramSlotDesc>& slots
    ) override {
        auto nativeShaderProgram = m_NativeRender->createShaderProgram(vertexFile, pixelFile, slots);
        return std::make_shared<DxShaderProgramWrapper>(nativeShaderProgram);
    }

    virtual std::shared_ptr<CrossPlatformComputeProgram> createComputeProgram(
        const std::string& file, const std::vector<ShaderProgramSlotDesc>& slots
    ) override {
        auto nativeComputeProgram = m_NativeRender->createComputeProgram(file, slots);
        return std::make_shared<DxComputeProgramWrapper>(nativeComputeProgram);
    }

    std::shared_ptr<CrossPlatformShaderProgramDataBuffer> createShaderProgramDataBuffer(size_t byteSize) override {
        auto nativeBuffer = m_NativeRender->createShaderProgramDataBuffer(byteSize);
        return std::make_shared<DxShaderProgramDataBufferWrapper>(nativeBuffer);
    }

    std::shared_ptr<CrossPlatformReadWriteDataBuffer> createReadWriteDataBuffer(size_t byteSize) override {
        auto nativeBuffer = m_NativeRender->createReadWriteDataBuffer(byteSize);
        return std::make_shared<DxReadWriteDataBufferWrapper>(nativeBuffer);
    }

    std::shared_ptr<CrossPlatformFramebuffer> createFramebuffer() override {
        auto nativeFB = std::make_shared<DxFramebuffer>();
        return std::make_shared<DxFramebufferWrapper>(nativeFB);
    }

    std::shared_ptr<CrossPlatformRenderPass> createRenderPass(
        std::shared_ptr<CrossPlatformShaderProgram> shaderProgram, CrossPlatformRenderPass::PipelineDesc pipelineDesc
    ) override {
        auto shaderProgramWrapper = std::static_pointer_cast<DxShaderProgramWrapper>(shaderProgram);

        DxRenderPass::PipelineDesc nativePipelineDesc;
        nativePipelineDesc.cullMode        = getDxCullMode(pipelineDesc.cullMode);
        nativePipelineDesc.depthFunc       = getDxDepthFunc(pipelineDesc.depthFunc);
        nativePipelineDesc.depthClipEnable = pipelineDesc.depthClipEnable;

        auto nativeRenderPass = m_NativeRender->createRenderPass(shaderProgramWrapper->getNative(), nativePipelineDesc);
        return std::make_shared<DxRenderPassWrapper>(nativeRenderPass);
    }

    std::shared_ptr<CrossPlatformRenderPass> createRenderPass(
        std::shared_ptr<CrossPlatformShaderProgram> shaderProgram,
        std::vector<CROSS_PLATFROM_TEXTURE_FORMATS> rtvFormats, CROSS_PLATFROM_TEXTURE_FORMATS dsvFormat,
        CrossPlatformRenderPass::PipelineDesc pipelineDesc
    ) override {
        auto shaderProgramWrapper = std::static_pointer_cast<DxShaderProgramWrapper>(shaderProgram);

        std::vector<DXGI_FORMAT> nativeRtvFormats;
        nativeRtvFormats.reserve(rtvFormats.size());
        for (CROSS_PLATFROM_TEXTURE_FORMATS format : rtvFormats) {
            nativeRtvFormats.push_back(getDxTextureFormat(format));
        }

        DXGI_FORMAT nativeDsvFormat = getDxTextureFormat(dsvFormat);

        DxRenderPass::PipelineDesc nativePipelineDesc;
        nativePipelineDesc.cullMode        = getDxCullMode(pipelineDesc.cullMode);
        nativePipelineDesc.depthFunc       = getDxDepthFunc(pipelineDesc.depthFunc);
        nativePipelineDesc.depthClipEnable = pipelineDesc.depthClipEnable;

        auto nativeRenderPass = m_NativeRender->createRenderPass(
            shaderProgramWrapper->getNative(), nativeRtvFormats, nativeDsvFormat, nativePipelineDesc
        );
        return std::make_shared<DxRenderPassWrapper>(nativeRenderPass);
    }

    std::shared_ptr<CrossPlatformComputePass> createComputePass(
        std::shared_ptr<CrossPlatformComputeProgram> computeProgram
    ) override {
        auto computerProgramWrapper = std::static_pointer_cast<DxComputeProgramWrapper>(computeProgram);
        auto nativeRenderPass       = m_NativeRender->createComputePass(computerProgramWrapper->getNative());
        return std::make_shared<DxComputePassWrapper>(nativeRenderPass);
    }

    void drawItem(const std::string& geometry, const std::string& subGeometry) override {
        m_NativeRender->drawItem(geometry, subGeometry);
    }

    void getViewport(uint32_t& width, uint32_t& height) override { m_NativeRender->getViewport(width, height); }

    void release() override { m_NativeRender->flushCommandQueue(); }

private:
    std::shared_ptr<DxRender> m_NativeRender;
};

}  // namespace Engine