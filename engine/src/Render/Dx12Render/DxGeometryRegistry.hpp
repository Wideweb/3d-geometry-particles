#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "DxMeshGeometry.hpp"
#include "DxUtils.hpp"
#include "Mesh.hpp"

namespace Engine {

class DxGeometryRegistry {
public:
    DxGeometryRegistry(ID3D12Device* device);

    const DxMeshGeometry* get(const std::string& geometry) const;

    void add(
        const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes,
        ID3D12GraphicsCommandList* commandList
    );

private:
    ID3D12Device*                                                    m_Device;
    std::unordered_map<std::string, std::unique_ptr<DxMeshGeometry>> m_Data;
};

}  // namespace Engine