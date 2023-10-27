#pragma once

#include "RenderTexture.hpp"
#include "DepthStencilTexture.hpp"
#include "DxUtils.hpp"

class Framebuffer {
public:
    Framebuffer() noexcept;

    void addAttachment(std::shared_ptr<RenderTexture> attachment);

    void setDSAttachment(std::shared_ptr<DepthStencilTexture> attachment);

    void resize(size_t width, size_t height);

    void beginRenderTo(ID3D12GraphicsCommandList* commandList);

    void endRenderTo(ID3D12GraphicsCommandList* commandList);

    void clear(ID3D12GraphicsCommandList* commandList);

    const std::vector<std::shared_ptr<RenderTexture>>& getAttachments() const noexcept { return m_Attachments; }

    std::shared_ptr<DepthStencilTexture> getDSAttachment() const noexcept { return m_DSAttachment; }

private:
    size_t                                                 m_Width;
    size_t                                                 m_Height;

    std::vector<std::shared_ptr<RenderTexture>>            m_Attachments;
    std::shared_ptr<DepthStencilTexture>                   m_DSAttachment;

    bool                                                   m_RenderTo;
};