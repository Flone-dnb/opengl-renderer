#include "Mesh.h"

// Standard.
#include <format>

// Custom.
#include "window/GLFW.hpp"
#include "shader/ShaderUniformHelpers.hpp"
#include "import/TextureImporter.h"

void Vertex::setVertexAttributes() {
    // Prepare offsets of fields.
    const auto iPositionOffset = offsetof(Vertex, position);
    const auto iNormalOffset = offsetof(Vertex, normal);
    const auto iUvOffset = offsetof(Vertex, uv);
    const auto iTangentOffset = offsetof(Vertex, tangent);

    // Specify position.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                                         // attribute index (layout location)
        3,                                         // number of components
        GL_FLOAT,                                  // type of component
        GL_FALSE,                                  // whether data should be normalized or not
        sizeof(Vertex),                            // stride (size in bytes between elements)
        reinterpret_cast<void*>(iPositionOffset)); // NOLINT: beginning offset

    // Specify normal.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                                       // attribute index (layout location)
        3,                                       // number of components
        GL_FLOAT,                                // type of component
        GL_FALSE,                                // whether data should be normalized or not
        sizeof(Vertex),                          // stride (size in bytes between elements)
        reinterpret_cast<void*>(iNormalOffset)); // NOLINT: beginning offset

    // Specify UV.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,                                   // attribute index (layout location)
        2,                                   // number of components
        GL_FLOAT,                            // type of component
        GL_FALSE,                            // whether data should be normalized or not
        sizeof(Vertex),                      // stride (size in bytes between elements)
        reinterpret_cast<void*>(iUvOffset)); // NOLINT: beginning offset

    // Specify tangent.
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,                                        // attribute index (layout location)
        3,                                        // number of components
        GL_FLOAT,                                 // type of component
        GL_FALSE,                                 // whether data should be normalized or not
        sizeof(Vertex),                           // stride (size in bytes between elements)
        reinterpret_cast<void*>(iTangentOffset)); // NOLINT: beginning offset
}

Mesh::~Mesh() {
    // Don't need to wait for the GPU to finish using this data because:
    // When a buffer, texture, sampler, renderbuffer, query, or sync object is deleted, its name immediately
    // becomes invalid (e.g. is marked unused), but the underlying object will not be deleted until it is no
    // longer in use.

    // Delete vertex buffer objects.
    glDeleteBuffers(1, &iVertexBufferObjectId);
    glDeleteVertexArrays(1, &iVertexArrayObjectId);

    // Delete index buffer.
    glDeleteBuffers(1, &iIndexBufferObjectId);

    // Delete textures.
    glDeleteTextures(1, &material.iDiffuseTextureId);
    glDeleteTextures(1, &material.iMetallicRoughnessTextureId);
    glDeleteTextures(1, &material.iEmissionTextureId);
    glDeleteTextures(1, &material.iNormalTextureId);

#if defined(DEBUG)
    static_assert(sizeof(Mesh) == 184, "add new resources to be deleted"); // NOLINT
#endif
}

std::unique_ptr<Mesh> Mesh::create(std::vector<Vertex>&& vVertices, std::vector<unsigned int>&& vIndices) {
    static_assert(sizeof(vIndices[0]) == sizeof(unsigned int), "change index format in the `draw` command");

    // Prepare the resulting mesh.
    auto pMesh = std::make_unique<Mesh>();

    // Prepare vertex/index buffers.
    pMesh->prepareVertexBuffer(std::move(vVertices));
    pMesh->prepareIndexBuffer(std::move(vIndices));

    // Prepare normal matrix.
    pMesh->normalMatrix = getNormalMatrixFromWorldMatrix(pMesh->worldMatrix);

    return pMesh;
}

void Mesh::setDiffuseTexture(const std::filesystem::path& pathToImageFile) {
    // Delete previous texture.
    glDeleteTextures(1, &material.iDiffuseTextureId);

    // Create new texture.
    material.iDiffuseTextureId = TextureImporter::loadTexture(pathToImageFile, true);
}

void Mesh::setNormalTexture(const std::filesystem::path& pathToImageFile) {
    // Delete previous texture.
    glDeleteTextures(1, &material.iNormalTextureId);

    // Create new texture.
    material.iNormalTextureId = TextureImporter::loadTexture(pathToImageFile, false);
}

void Mesh::setMetallicRoughnessTexture(const std::filesystem::path& pathToImageFile) {
    // Delete previous texture.
    glDeleteTextures(1, &material.iMetallicRoughnessTextureId);

    // Create new texture.
    material.iMetallicRoughnessTextureId = TextureImporter::loadTexture(pathToImageFile, false);
}

void Mesh::setEmissionTexture(const std::filesystem::path& pathToImageFile) {
    // Delete previous texture.
    glDeleteTextures(1, &material.iEmissionTextureId);

    // Create new texture.
    material.iEmissionTextureId = TextureImporter::loadTexture(pathToImageFile, false);
}

void Mesh::setWorldMatrix(const glm::mat4x4& newWorldMatrix) {
    // Save new world matrix.
    worldMatrix = newWorldMatrix;

    // Update normal matrix.
    normalMatrix = getNormalMatrixFromWorldMatrix(worldMatrix);
}

glm::mat4x4* Mesh::getWorldMatrix() { return &worldMatrix; }

glm::mat3x3* Mesh::getNormalMatrix() { return &normalMatrix; }

glm::mat3x3 Mesh::getNormalMatrixFromWorldMatrix(const glm::mat4x4& worldMatrix) {
    return glm::mat3x3(glm::transpose(glm::inverse(worldMatrix)));
}

void Material::setTexture2dParameters() {
    // Set texture wrapping.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set texture filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR); // no need to set `MIPMAP` option since magnification does not use mipmaps

    // Enable anisotropic texture filtering (core in OpenGL 4.6 which we are using).
    float maxSupportedAnisotropy = 0.0F;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxSupportedAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxSupportedAnisotropy);
}

void Mesh::prepareVertexBuffer(std::vector<Vertex>&& vVertices) {
    // Create vertex array object (VAO).
    glGenVertexArrays(1, &iVertexArrayObjectId);

    // Bind our vertex array object to the OpenGL context.
    glBindVertexArray(iVertexArrayObjectId);

    // Create vertex buffer object (VBO).
    glGenBuffers(1, &iVertexBufferObjectId);

    // Set our vertex buffer to the "array" target  in OpenGL context to update its data.
    glBindBuffer(GL_ARRAY_BUFFER, iVertexBufferObjectId);

    // Copy vertices to the buffer.
    glBufferData(
        GL_ARRAY_BUFFER,
        vVertices.size() * sizeof(vVertices[0]),
        vVertices.data(),
        GL_STATIC_DRAW); // `STATIC` because the data will not be changed

    // Describe vertex attributes.
    Vertex::setVertexAttributes();

    // Finished with vertex buffer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate AABB.
    aabb = AABB::createFromVertices(&vVertices);
}

void Mesh::prepareIndexBuffer(std::vector<unsigned int>&& vIndices) {
    // Make sure we don't exceed type limit.
    constexpr size_t iTypeLimit = std::numeric_limits<int>::max();
    if (vIndices.size() > iTypeLimit) [[unlikely]] {
        throw std::runtime_error(
            std::format("index count {} exceeds type limit of {}", vIndices.size(), iTypeLimit));
    }

    // Save index count.
    iIndexCount = static_cast<int>(vIndices.size());

    // Create element buffer object.
    glGenBuffers(1, &iIndexBufferObjectId);

    // Set our index buffer to the "element array" target in OpenGL context to update its data.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIndexBufferObjectId);

    // Copy indices to the buffer.
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        vIndices.size() * sizeof(vIndices[0]),
        vIndices.data(),
        GL_STATIC_DRAW); // `STATIC` because the data will not be changed

    // Finished with index buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Material::setToShader(unsigned int iShaderProgramId) const {
    // Set diffuse texture at texture unit (location) 0.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, iDiffuseTextureId);
    setTexture2dParameters();

    // Set normal texture at texture unit (location) 1.
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, iNormalTextureId);
    setTexture2dParameters();

    // Set metallic+roughness texture at texture unit (location) 2.
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, iMetallicRoughnessTextureId);
    setTexture2dParameters();

    // Set emission texture at texture unit (location) 3.
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, iEmissionTextureId);
    setTexture2dParameters();

    // Set diffuse color.
    ShaderUniformHelpers::setVector3ToShader(iShaderProgramId, "material.diffuseColor", diffuseColor);

    // Set specular color.
    ShaderUniformHelpers::setVector3ToShader(iShaderProgramId, "material.specularColor", specularColor);

    // Set shininess.
    ShaderUniformHelpers::setFloatToShader(iShaderProgramId, "material.shininess", shininess);
}
