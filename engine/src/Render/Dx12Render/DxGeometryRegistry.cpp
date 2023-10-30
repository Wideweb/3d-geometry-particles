#include "DxGeometryRegistry.hpp"

namespace Engine {

DxGeometryRegistry::DxGeometryRegistry(ID3D12Device* device) : m_Device(device)  { }

const DxMeshGeometry* DxGeometryRegistry::get(const std::string& geometry) const {
    return m_Data.at(geometry).get();
}

void DxGeometryRegistry::add(
    const std::string& geometry,
    const std::vector<std::string>& subGeometries,
    const std::vector<Mesh>& subMeshes
    ID3D12GraphicsCommandList* commandList
) {
    auto geo = std::make_unique<DxMeshGeometry>();
    geo->Name = geometry;
    
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    for (size_t i = 0; i < subGeometries.size(); i++) {
        auto& id = subGeometries[i];
        auto& mesh = subMeshes[i];

        SubmeshGeometry subGeo;
        subGeo.indexCount = (UINT)mesh.indices.size();
        subGeo.startIndexLocation = indices.size();
        subGeo.baseVertexLocation = vertices.size();

        indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
        vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        geo->drawArgs[id] = subGeo;
    }

    const size_t vbByteSize = vertices.size() * sizeof(Vertex);
    const size_t ibByteSize = indices.size() * sizeof(uint16_t);

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->vertexBufferCPU));
    CopyMemory(geo->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->vertexBufferGPU = DxUtils::CreateDefaultBuffer(m_Device,
        m_CommandList, vertices.data(), vbByteSize, geo->vertexBufferUploader);

    geo->indexBufferGPU = DxUtils::CreateDefaultBuffer(m_Device,
        m_CommandList, indices.data(), ibByteSize, geo->indexBufferUploader);

    geo->vertexByteStride = sizeof(Vertex);
    geo->vertexBufferByteSize = vbByteSize;
    geo->indexFormat = DXGI_FORMAT_R16_UINT;
    geo->indexBufferByteSize = ibByteSize;

    m_Data[geo->Name] = std::move(geo);
}

} // namespace Engine