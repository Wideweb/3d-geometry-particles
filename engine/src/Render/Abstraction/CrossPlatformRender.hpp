#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Mesh.hpp"
#include "ShaderProgramSlot.hpp"

namespace Engine {

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// CULL FUNC /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
enum class CULL_MODE {
    NONE,
    BACK,
    FRONT
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DEPTH FUNC ////////////////////////////////
////////////////////////////////////////////////////////////////////////////
enum DEPTH_FUNC {
    NEVER	      = 1,
    LESS	      = 2,
    EQUAL	      = 3,
    LESS_EQUAL	  = 4,
    GREATER	      = 5,
    NOT_EQUAL	  = 6,
    GREATER_EQUAL = 7,
    ALWAYS	      = 8
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////// TEXTURE FORMAT //////////////////////////////
////////////////////////////////////////////////////////////////////////////
enum class CROSS_PLATFROM_TEXTURE_FORMATS { 
    RGBA8 = 1,
    D24S8 = 2
};

////////////////////////////////////////////////////////////////////////////
///////////////////////////////// TEXTURE //////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformTexture { };

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
/////////////////////// SHADER PROGRAM DATA BUFFER /////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformShaderProgramDataBuffer {
public:
    virtual void copyData(void* data) = 0;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////// SHADER PROGRAM //////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformShaderProgram {
public:
    virtual void setDataSlot(size_t index, std::shared_ptr<CrossPlatformShaderProgramDataBuffer> buffer) = 0;
    virtual void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformTexture> texture) = 0;
    virtual void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformRenderTexture> texture) = 0;
    virtual void setTextureSlot(size_t index, std::shared_ptr<CrossPlatformDepthStencilTexture> texture) = 0;
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// RENDER PASS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
class CrossPlatformRenderPass {
public:
    struct PipelineDesc {
        CULL_MODE cullMode = CULL_MODE::BACK;
        DEPTH_FUNC depthFunc = DEPTH_FUNC::LESS;
        bool depthClipEnable = true;
    };
};

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

    virtual void beginInitialization() = 0;

    virtual void endInitialization() = 0;

    virtual void beginFrame() = 0;

    virtual void endFrame() = 0;

    virtual void resize(uint32_t width, uint32_t height) = 0;

    virtual void clear(float r, float g, float b, float a) = 0;

    virtual void setRenderPass(std::shared_ptr<CrossPlatformRenderPass> pass) = 0;

    virtual void setFramebuffer(std::shared_ptr<CrossPlatformFramebuffer> fb) = 0;

    virtual void registerGeometry(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes) = 0;

    virtual std::shared_ptr<CrossPlatformTexture> loadTexture(const std::string& filename) = 0;

    virtual std::shared_ptr<CrossPlatformTexture> loadCubeTexture(const std::array<std::string, 6> &files) = 0;

    virtual std::shared_ptr<CrossPlatformDepthStencilTexture> createDepthStencilTexture(size_t width, size_t height) = 0;

    virtual std::shared_ptr<CrossPlatformRenderTexture> createRenderTexture(CROSS_PLATFROM_TEXTURE_FORMATS format, size_t width, size_t height) = 0;

    virtual std::shared_ptr<CrossPlatformShaderProgram> createShaderProgram(const std::string& vertexFile, const std::string& pixelFile, const std::vector<ShaderProgramSlotDesc>& slots) = 0;

    virtual std::shared_ptr<CrossPlatformShaderProgramDataBuffer> createShaderProgramDataBuffer(size_t byteSize) = 0;

    virtual std::shared_ptr<CrossPlatformFramebuffer> createFramebuffer() = 0;

    virtual std::shared_ptr<CrossPlatformRenderPass> createRenderPass(
        std::shared_ptr<CrossPlatformShaderProgram> shaderProgram,
        CrossPlatformRenderPass::PipelineDesc pipelineDesc
    ) = 0;

    virtual std::shared_ptr<CrossPlatformRenderPass> createRenderPass(
        std::shared_ptr<CrossPlatformShaderProgram> shaderProgram,
        std::vector<CROSS_PLATFROM_TEXTURE_FORMATS> rtvFormats,
        CROSS_PLATFROM_TEXTURE_FORMATS dsvFormat,
        CrossPlatformRenderPass::PipelineDesc pipelineDesc
    ) = 0;
    
    virtual void drawItem(const std::string& geometry, const std::string& subGeometry) = 0;

    virtual void getViewport(uint32_t& width, uint32_t& height) = 0;

    virtual void release() = 0;

    static CrossPlatformRender* create(void* window, uint32_t width, uint32_t height);
};

} // namespace Engine