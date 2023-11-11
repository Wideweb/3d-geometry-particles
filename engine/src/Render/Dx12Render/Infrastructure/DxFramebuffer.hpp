#pragma once

#include "DxDepthStencilTexture.hpp"
#include "DxRenderTexture.hpp"
#include "DxUtils.hpp"

namespace Engine {

class DxFramebuffer {
public:
    DxFramebuffer() noexcept;

    void addAttachment(std::shared_ptr<DxRenderTexture> attachment);

    void setDSAttachment(std::shared_ptr<DxDepthStencilTexture> attachment);

    void resize(size_t width, size_t height);

    void beginRenderTo(ID3D12GraphicsCommandList* commandList);

    void endRenderTo(ID3D12GraphicsCommandList* commandList);

    void clear(ID3D12GraphicsCommandList* commandList);

    const std::vector<std::shared_ptr<DxRenderTexture>>& getAttachments() const noexcept { return m_Attachments; }

    std::shared_ptr<DxDepthStencilTexture> getDSAttachment() const noexcept { return m_DSAttachment; }

private:
    size_t m_Width;
    size_t m_Height;

    std::vector<std::shared_ptr<DxRenderTexture>> m_Attachments;
    std::shared_ptr<DxDepthStencilTexture>        m_DSAttachment;

    bool m_RenderTo;
};

}  // namespace Engine