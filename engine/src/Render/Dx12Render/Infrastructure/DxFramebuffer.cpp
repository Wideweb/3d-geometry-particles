#include "DxFramebuffer.hpp"

namespace Engine {

DxFramebuffer::DxFramebuffer() noexcept
: m_RenderTo(false),
  m_Width(0),
  m_Height(0) {}

void DxFramebuffer::addAttachment(std::shared_ptr<DxRenderTexture> attachment) { m_Attachments.push_back(attachment); }

void DxFramebuffer::setDSAttachment(std::shared_ptr<DxDepthStencilTexture> attachment) { m_DSAttachment = attachment; }

void DxFramebuffer::resize(size_t width, size_t height) {
    if (m_Width == width && m_Height == height) {
        return;
    }

    for (auto& rt : m_Attachments) {
        rt->resize(width, height);
    }

    m_DSAttachment->resize(width, height);

    m_Width  = width;
    m_Height = height;
}

void DxFramebuffer::beginRenderTo(ID3D12GraphicsCommandList* commandList) {
    if (m_RenderTo) {
        return;
    }

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvDescriptors;
    for (auto& rt : m_Attachments) {
        rt->beginRenderTo(commandList);
        rtvDescriptors.push_back(rt->getRtvDescriptor().cpu);
    }

    m_DSAttachment->beginRenderTo(commandList);

    auto dsDescriptor = m_DSAttachment->getDsvDescriptor().cpu;
    commandList->OMSetRenderTargets(rtvDescriptors.size(), rtvDescriptors.data(), true, &dsDescriptor);

    m_RenderTo = true;
}

void DxFramebuffer::endRenderTo(ID3D12GraphicsCommandList* commandList) {
    if (!m_RenderTo) {
        return;
    }

    for (auto& rt : m_Attachments) {
        rt->endRenderTo(commandList);
    }

    m_DSAttachment->endRenderTo(commandList);

    m_RenderTo = false;
}

void DxFramebuffer::clear(ID3D12GraphicsCommandList* commandList) {
    m_DSAttachment->clear(commandList);
    for (auto& rt : m_Attachments) {
        rt->clear(commandList);
    }
}

}  // namespace Engine