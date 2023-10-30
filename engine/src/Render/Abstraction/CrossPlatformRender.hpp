#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Mesh.hpp"

namespace Engine {

////////////////////////////////////////////////////////////////////////////
////////////////////////////// TEXTURE FORMAT //////////////////////////////
////////////////////////////////////////////////////////////////////////////
enum class CROSS_PLATFROM_TEXTURE_FORMATS {
    RGBA8
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////// RENDER TEXTURE //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformRenderTexture {
public:
    virtual void resize(size_t width, size_t height) = 0;
};

////////////////////////////////////////////////////////////////////////////
/////////////////////////// DEPTH STENCIL TEXTURE //////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformDepthStencilTexture {
public:
    virtual void resize(size_t width, size_t height) = 0;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////// SHADER PROGRAM //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformShaderProgram {
public:
    virtual void setDataSlot(size_t index, void* data) = 0;
    virtual void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformRenderTexture> texture) = 0;
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// RENDER PASS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformRenderPass { };

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FRAMEBUFFER ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformFramebuffer {
public:
    virtual void addAttachment(std::shared_ptr<CrossPlatformRenderTexture> attachment) = 0;

    virtual void setDSAttachment(std::shared_ptr<CrossPlatformDepthStencilTexture> attachment) = 0;

    virtual void resize(size_t width, size_t height) = 0;

    virtual const std::vector<std::shared_ptr<CrossPlatformRenderTexture>>& getAttachments() const noexcept = 0;

    virtual std::shared_ptr<CrossPlatformDepthStencilTexture> getDSAttachment() const noexcept = 0;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////// RENDER //////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformRender {
public:
    virtual ~CrossPlatformRender() = default;

    virtual void beginFrame() = 0;

    virtual void endFrame() = 0;

    virtual void resize(uint32_t width, uint32_t height) = 0;

    virtual void clear(float r, float g, float b, float a) = 0;

    virtual void setRenderPass(std::shared_ptr<CrossPlatformRenderPass> pass) = 0;

    virtual void setFramebuffer(std::shared_ptr<CrossPlatformFramebuffer> fb) = 0;

    virtual void registerGeometry(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes) = 0;

    virtual std::shared_ptr<CrossPlatformDepthStencilTexture> createDepthStencilTexture(size_t width, size_t height) = 0;

    virtual std::shared_ptr<CrossPlatformRenderTexture> createRenderTexture(CROSS_PLATFROM_TEXTURE_FORMATS format, size_t width, size_t height) = 0;

    virtual std::shared_ptr<CrossPlatformShaderProgram> createShaderProgram(std::shared_ptr<DxShaderProgram> createShaderProgram(const std::string& vertexFile, const std::string& pixelFile, const std::vector<size_t>& dataSlots, const std::vector<std::string>& textureSlots) = 0;

    virtual std::shared_ptr<CrossPlatformFramebuffer> createFramebuffer() = 0;

    virtual std::shared_ptr<CrossPlatformRenderPass> createRenderPass(std::shared_ptr<CrossPlatformShaderProgram> shaderProgram) = 0;

    virtual std::shared_ptr<CrossPlatformRenderPass> createRenderPass(std::shared_ptr<CrossPlatformShaderProgram> shaderProgram, std::vector<CROSS_PLATFROM_TEXTURE_FORMATS> rtvFormats) = 0;
    
    virtual void drawItem(const std::string& geometry, const std::string& subGeometry) = 0;

    static CrossPlatformRender* create(void* window, uint32_t width, uint32_t height);
};

} // namespace Engine