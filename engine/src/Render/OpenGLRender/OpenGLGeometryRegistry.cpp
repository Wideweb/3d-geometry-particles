#include "OpenGLGeometryRegistry"

namespace Engine {

const OpenGLMeshGeometry* OpenGLGeometryRegistry::get(const std::string& geometry) const {
    return m_Data.at(geometry).get();
}

void OpenGLGeometryRegistry::add(
    const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes
) {
    auto geo  = std::make_unique<OpenGLMeshGeometry>();
    geo->Name = geometry;

    std::vector<Vertex>   vertices;
    std::vector<uint16_t> indices;

    for (size_t i = 0; i < subGeometries.size(); i++) {
        auto& id   = subGeometries[i];
        auto& mesh = subMeshes[i];

        OpenGLSubmeshGeometry subGeo;
        subGeo.indexCount          = mesh.indices.size();
        subGeo.startIndexLocation  = indices.size();
        subGeo.startVertexLocation = vertices.size();

        indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
        vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        geo->drawArgs[id] = subGeo;
    }

    const size_t vbByteSize = vertices.size() * sizeof(Vertex);
    const size_t ibByteSize = indices.size() * sizeof(uint16_t);

    glGenVertexArrays(1, &geo->VAO);
    glBindVertexArray(geo->VAO);

    glGenBuffers(1, &geo->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * static_cast<GLuint>(indices.size()), indices.data(), GL_STATIC_DRAW
    );

    glGenBuffers(1, &geo->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, geo->VBO);
    glBufferData(
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW
    );

    /////////////////////////////////////////////////////////////
    ///////////////////////// POSITION //////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    /////////////////////////////////////////////////////////////
    ////////////////////////// NORMAL ///////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    /////////////////////////////////////////////////////////////
    ////////////////////// TEXTURE COORD ////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    /////////////////////////////////////////////////////////////
    ///////////////////////// TANGENT ///////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    /////////////////////////////////////////////////////////////
    //////////////////////// BITANGENT //////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    /////////////////////////////////////////////////////////////
    ////////////////////////// COLOR ////////////////////////////
    /////////////////////////////////////////////////////////////
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(14 * sizeof(float)));
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    geo->vertexByteStride     = sizeof(Vertex);
    geo->vertexBufferByteSize = vbByteSize;
    geo->indexBufferByteSize  = ibByteSize;

    m_Data[geo->Name] = std::move(geo);
}

}  // namespace Engine