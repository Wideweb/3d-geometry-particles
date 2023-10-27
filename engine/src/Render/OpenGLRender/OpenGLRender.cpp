#include "OpenGLRender.hpp"

namespace Engine {

OpenGLRender::OpenGLRender(void* window, uint32_t width, uint32_t height) : m_Window(window), m_Width(width), m_Height(height) {
    
}

void OpenGLRender::beginFrame() {
    
}

void OpenGLRender::endFrame() {
    
}


OpenGLRender::~OpenGLRender() {

}

void OpenGLRender::setRenderPass(std::shared_ptr<OpenGLRenderPass> pass) {
    if (m_RenderPass != nullptr) {
        m_RenderPass->unbind();
    }

    if (pass != nullptr) {
        pass->bind();
    }

    m_RenderPass = pass;
}

void OpenGLRender::setFramebuffer(std::shared_ptr<Framebuffer> fb) {
    if (m_Framebuffer != nullptr) {
        m_Framebuffer->endRenderTo();
    }

    if (fb == nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        m_Framebuffer->beginRenderTo();
    }

    m_Framebuffer = fb;
}

void OpenGLRender::clear(float r, float g, float b, float a) {
    const float clearColor[] = { r, g, b, a };

    if (m_Framebuffer == nullptr) {
        m_CommandList->ClearRenderTargetView(currentBackBuffer(), clearColor, 0, nullptr);
        m_CommandList->ClearDepthStencilView(depthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    } else {
        m_Framebuffer->clear(m_CommandList.Get(), clearColor);
    }
}

void OpenGLRender::registerGeometry(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes) {
	m_GeometryRegistry->add(geometry, subGeometries, subMeshes);
}

void OpenGLRender::setShaderParameterTexture(size_t index, GLuint resource) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, resource);
    glUniform1i(location, index);
}

void OpenGLRender::drawItem(const std::string& geometry, const std::string& subGeometry) {
    MeshGeometry* geo = geoRegistry->get(geometry);
    SubmeshGeometry& subGeo = geo->drawArgs[subGeometry];

    m_CommandList->IASetVertexBuffers(0, 1, &geo->vertexBufferView());
    m_CommandList->IASetIndexBuffer(&geo->indexBufferView());
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    uint32_t indexCountPerInstance = subGeo.indexCount;
    uint32_t instanceCount = 1;
    uint32_t startIndexLocation = subGeo.startIndexLocation;
    uint32_t baseVertexLocation = subGeo.baseVertexLocation;
    uint32_t startInstanceLocation = 0;

    m_CommandList->DrawIndexedInstanced(
        indexCountPerInstance,
        instanceCount,
        startIndexLocation,
        baseVertexLocation,
        startInstanceLocation
    );
}

} // namespace Engine
