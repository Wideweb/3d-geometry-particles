#include "Framebuffer.hpp"

Framebuffer::Framebuffer() noexcept :
    m_RenderTo(false),
    m_Width(0),
    m_Height(0) {
}

void Framebuffer::addAttachment(std::shared_ptr<RenderTexture> attachment) {
    m_Attachments.push_back(attachment);
}

void Framebuffer::setDSAttachment(std::shared_ptr<DepthStencilTexture> attachment) {
    m_DSAttachment = attachment;
}

void Framebuffer::resize(size_t width, size_t height) {
    if (m_Width == width && m_Height == height) {
        return;
    }

    for (auto& rt: m_Attachments) {
        rt->resize(width, height);
    }

    m_DSAttachment->resize(width, height);

    m_Width = width;
    m_Height = height;
}

void Framebuffer::beginRenderTo(ID3D12GraphicsCommandList* commandList) {
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvDescriptors;
    for (auto& rt: m_Attachments) {
        rt->beginRenderTo(commandList);
        rtvDescriptors.push_back(rt->getRtvDescriptor());
    }

    m_DSAttachment->beginRenderTo(commandList);

    commandList->OMSetRenderTargets(rtvDescriptors.size(), rtvDescriptors.data(), true, &m_DSAttachment->getDsvDescriptor());

    m_RenderTo = true;
}

void Framebuffer::endRenderTo(ID3D12GraphicsCommandList* commandList) {
    for (auto& rt: m_Attachments) {
        rt->endRenderTo(commandList);
    }

    m_DSAttachment->endRenderTo(commandList);

    m_RenderTo = false;
}

void Framebuffer::clear(ID3D12GraphicsCommandList* commandList) {
    m_DSAttachment->clear(commandList);
    for (auto& rt: m_Attachments) {
        rt->clear(commandList);
    }
}