#include "DxGeometryRegistry.hpp"

namespace Engine {

DxGeometryRegistry::DxGeometryRegistry(ID3D12Device* device)
: m_Device(device) {}

const DxMeshGeometry* DxGeometryRegistry::get(const std::string& geometry) const { return m_Data.at(geometry).get(); }

void DxGeometryRegistry::add(
    const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes,
    ID3D12GraphicsCommandList* commandList
) {
    auto geo  = std::make_unique<DxMeshGeometry>();
    geo->name = geometry;

    std::vector<Vertex>   vertices;
    std::vector<uint16_t> indices;

    for (size_t i = 0; i < subGeometries.size(); i++) {
        auto& id   = subGeometries[i];
        auto& mesh = subMeshes[i];

        DxSubmeshGeometry subGeo;
        subGeo.indexCount         = static_cast<UINT>(mesh.indices.size());
        subGeo.startIndexLocation = static_cast<UINT>(indices.size());
        subGeo.baseVertexLocation = static_cast<UINT>(vertices.size());

        indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
        vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        geo->drawArgs[id] = subGeo;
    }

    const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(uint16_t));

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->vertexBufferCPU));
    CopyMemory(geo->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->indexBufferCPU));
    CopyMemory(geo->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->vertexBufferGPU =
        DxUtils::CreateDefaultBuffer(m_Device, commandList, vertices.data(), vbByteSize, geo->vertexBufferUploader);

    geo->indexBufferGPU =
        DxUtils::CreateDefaultBuffer(m_Device, commandList, indices.data(), ibByteSize, geo->indexBufferUploader);

    geo->vertexByteStride     = sizeof(Vertex);
    geo->vertexBufferByteSize = vbByteSize;
    geo->indexFormat          = DXGI_FORMAT_R16_UINT;
    geo->indexBufferByteSize  = ibByteSize;

    m_Data[geo->name] = std::move(geo);
}

}  // namespace Engine