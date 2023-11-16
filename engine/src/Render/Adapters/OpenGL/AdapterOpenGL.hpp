#pragma once

#include "CrossPlatformRender.hpp"
#include "OpenGLFramebuffer.hpp"
#include "OpenGLRenderPass.hpp"
#include "OpenGLRenderTexture.hpp"
#include "OpenGLShaderProgram.hpp"

namespace Engine {

////////////////////////////////////////////////////////////////////////////
////////////////////////////// TEXTURE FORMAT //////////////////////////////
////////////////////////////////////////////////////////////////////////////
unsigned int getOpenGLTextureFormat(CROSS_PLATFROM_TEXTURE_FORMATS format) {
    switch (format) {
        case CROSS_PLATFROM_TEXTURE_FORMATS::RGBA8:
            return GL_RGBA8;
        default:
            throw std::invalid_argument("GfxImage::getOpenGLTextureFormat: invalid internal format.");
    }
}

////////////////////////////////////////////////////////////////////////////
///////////////////////////////// TEXTURE //////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class OpenGLTextureWrapper : public CrossPlatformTexture {
public:
    OpenGLTextureWrapper(std::shared_ptr<OpenGLTexture> nativeTexture) { m_NativeTexture = nativeTexture; }

    std::shared_ptr<OpenGLTexture> getNative() { return m_NativeTexture; }

private:
    std::shared_ptr<OpenGLTexture> m_NativeTexture;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////// RENDER TEXTURE //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class OpenGLRenderTextureWrapper : public CrossPlatformRenderTexture {
public:
    OpenGLRenderTextureWrapper(std::shared_ptr<OpenGLRenderTexture> nativeRT) { m_NativeRT = nativeRT; }

    void resize(size_t width, size_t height) override { m_NativeRT->resize(width, height); }

    std::shared_ptr<OpenGLRenderTexture> getNative() { return m_NativeRT; }

private:
    std::shared_ptr<OpenGLRenderTexture> m_NativeRT;
};

////////////////////////////////////////////////////////////////////////////
/////////////////////////// DEPTH STENCIL TEXTURE //////////////////////////
////////////////////////////////////////////////////////////////////////////
class OpenGLDepthStencilTextureWrapper : public CrossPlatformDepthStencilTexture {
public:
    OpenGLDepthStencilTextureWrapper(std::shared_ptr<OpenGLDepthStencilTexture> nativeRT) { m_NativeRT = nativeRT; }

    void resize(size_t width, size_t height) override { nativeRT->resize(width, height); }

    std::shared_ptr<OpenGLDepthStencilTexture> getNative() { return m_NativeRT; }

private:
    std::shared_ptr<OpenGLDepthStencilTexture> m_NativeRT;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////// SHADER PROGRAM //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class OpenGLShaderProgramWrapper : public CrossPlatformShaderProgram {
public:
    OpenGLShaderProgramWrapper(std::shared_ptr<OpenGLShaderProgram> nativeSP) noexcept { m_NativeSP = nativeSP; }

    void setDataSlot(size_t index, void* data) override { m_NativeSP->setDataSlot(index, data); }

    void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformRenderTexture> texture) override {
        auto textureWrapper = std::static_pointer_cast<OpenGLRenderTextureWrapper>(texture);
        m_NativeSP->setTextureSlot(index, textureWrapper->getNative());
    }

private:
    std::shared_ptr<OpenGLShaderProgram> m_NativeSP;
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// RENDER PASS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class OpenGLRenderPassWrapper : CrossPlatformRenderPass {
public:
    OpenGLRenderPassWrapper(std::shared_ptr<OpenGLRenderPass> nativeRP) noexcept { m_NativeRP = nativeRP; }

    std::shared_ptr<OpenGLRenderPass> getNative() { return m_NativeRP; }

private:
    std::shared_ptr<OpenGLShaderProgram> m_NativeRP;
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FRAMEBUFFER ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class OpenGLFramebufferWrapper : public CrossPlatformFramebuffer {
public:
    OpenGLFramebufferWrapper(std::shared_ptr<OpenGLFramebuffer> nativeFB) noexcept { m_NativeFB = nativeFB; }

    void addAttachment(std::shared_ptr<CrossPlatformRenderTexture> attachment) override {
        auto textureWrapper = std::static_pointer_cast<OpenGLRenderTextureWrapper>(attachment);
        m_NativeFB->addAttachment(textureWrapper->getNative());

        m_Attachments.push_back(attachment);
    }

    void setDSAttachment(std::shared_ptr<CrossPlatformDepthStencilTexture> attachment) override {
        auto dsTextureWrapper = std::static_pointer_cast<OpenGLDepthStencilTextureWrapper>(attachment);
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

private:
    std::shared_ptr<OpenGLFramebuffer>                       m_NativeFB;
    std::vector<std::shared_ptr<CrossPlatformRenderTexture>> m_Attachments;
    std::shared_ptr<CrossPlatformDepthStencilTexture>        m_DSAttachment;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////// RENDER //////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class OpenGLPlatformRenderWrapper : public CrossPlatformRender {
public:
    OpenGLPlatformRenderWrapper(void* window, uint32_t width, uint32_t height) {
        m_NativeRender = std::make_shared<OpenGLRender>(window, width, height);
    }

    virtual ~OpenGLPlatformRenderWrapper() = default;

    void beginInitialization() override {}

    void endInitialization() override {}

    void beginFrame() override { m_NativeRender->beginFrame(); }

    void endFrame() override { m_NativeRender->endFrame(); }

    void resize(uint32_t width, uint32_t height) override { m_NativeRender->resize(width, height); }

    void clear(float r, float g, float b, float a) override { m_NativeRender->clear(r, g, b, a); }

    void setPass(std::shared_ptr<CrossPlatformRenderPass> pass) override {
        auto passWrapper = std::static_pointer_cast<OpenGLRenderPassWrapper>(pass);
        m_NativeRender->setPass(passWrapper->getNative());
    }

    void setFramebuffer(std::shared_ptr<CrossPlatformFramebuffer> fb) override {
        auto fbWrapper = std::static_pointer_cast<OpenGLFramebufferWrapper>(fb);
        m_NativeRender->setFramebuffer(fbWrapper->getNative());
    }

    void registerGeometry(
        const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
    ) override {
        m_NativeRender->registerGeometry(geometry, subGeometries, subMeshes);
    }

    std::shared_ptr<CrossPlatformTexture> loadTexture(const std::string& filename) override {
        auto nativeTexture = m_NativeRender->loadTexture(filename);
        return std::make_shared<OpenGLTextureWrapper>(nativeTexture);
    }

    std::shared_ptr<CrossPlatformDepthStencilTexture> createDepthStencilTexture(size_t width, size_t height) override {
        auto nativeDSTexture = std::make_shared<OpenGLDepthStencilTexture>(width, height);
        return std::shared_ptr<OpenGLDepthStencilTextureWrapper>(nativeDSTexture);
    }

    std::shared_ptr<CrossPlatformRenderTexture> createRenderTexture(
        CROSS_PLATFROM_TEXTURE_FORMATS format, size_t width, size_t height
    ) override {
        auto nativeRT = std::make_shared<OpenGLRenderTexture>(getOpenGLTextureFormat(format), GL_FLOAT, width, height);
        return std::shared_ptr<OpenGLRenderTextureWrapper>(nativeRT);
    }

    std::shared_ptr<CrossPlatformShaderProgram> createShaderProgram(std::shared_ptr<DxShaderProgram> createShaderProgram(const std::string& vertexFile, const std::string& pixelFile, const std::vector<size_t>& dataSlots, const std::vector<std::string>& textureSlots) override {
        auto std::make_shared<OpenGLShaderProgram>(vertexFile, pixelFile, dataSlots, textureSlots);
        return std::shared_ptr<OpenGLRenderTextureWrapper>(nativeRT);
    }

    std::shared_ptr<CrossPlatformFramebuffer> createFramebuffer() override {
        auto nativeFB = std::make_shared<OpenGLFramebuffer>();
        return std::make_shared<OpenGLFramebufferWrapper>(nativeFB);
    }

    std::shared_ptr<CrossPlatformRenderPass> createRenderPass(std::shared_ptr<CrossPlatformShaderProgram> shaderProgram) override {
        auto shaderProgramWrapper = std::static_pointer_cast<OpenGLShaderProgramWrapper>(shaderProgram);
        return m_NativeRender->createRenderPass(shaderProgramWrapper->getNative());
    }

    std::shared_ptr<CrossPlatformRenderPass> createRenderPass(std::shared_ptr<CrossPlatformShaderProgram> shaderProgram, std::vector<CROSS_PLATFROM_TEXTURE_FORMATS> rtvFormats) override {
        auto shaderProgramWrapper = std::static_pointer_cast<OpenGLShaderProgramWrapper>(shaderProgram);
        return m_NativeRender->createRenderPass(shaderProgramWrapper->getNative());
    }
    
    void drawItem(const std::string& geometry, const std::string& subGeometry) override {
        m_NativeRender->drawItem(geometry, subGeometry);
    }

private:
    std::shared_ptr<OpenGLRender> m_NativeRender;
};

}  // namespace Engine