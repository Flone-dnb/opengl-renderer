#include "Mesh.h"

// Standard.
#include <format>

// Custom.
#include "GLFW.hpp"
#include "Application.h"

void Vertex::setVertexAttributes() {
    // Prepare offsets of fields.
    const auto iPositionOffset = offsetof(Vertex, position);
    const auto iUvOffset = offsetof(Vertex, uv);

    // Specify position.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                                         // attribute index (layout location)
        3,                                         // number of components
        GL_FLOAT,                                  // type of component
        GL_FALSE,                                  // whether data should be normalized or not
        sizeof(Vertex),                            // stride (size in bytes between elements)
        reinterpret_cast<void*>(iPositionOffset)); // NOLINT: beginning offset

    // Specify UV.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                                   // attribute index (layout location)
        2,                                   // number of components
        GL_FLOAT,                            // type of component
        GL_FALSE,                            // whether data should be normalized or not
        sizeof(Vertex),                      // stride (size in bytes between elements)
        reinterpret_cast<void*>(iUvOffset)); // NOLINT: beginning offset
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

    // Delete texture.
    glDeleteTextures(1, &iDiffuseTextureId);

#if defined(DEBUG)
    static_assert(sizeof(Mesh) == 108, "add new resources to be deleted"); // NOLINT
#endif
}

std::unique_ptr<Mesh> Mesh::create(std::vector<Vertex>&& vVertices, std::vector<unsigned int>&& vIndices) {
    static_assert(sizeof(vIndices[0]) == sizeof(unsigned int), "change index format in the `draw` command");

    // Prepare the resulting mesh.
    auto pMesh = std::make_unique<Mesh>();

    // Prepare vertex/index buffers.
    pMesh->prepareVertexBuffer(std::move(vVertices));
    pMesh->prepareIndexBuffer(std::move(vIndices));

    return pMesh;
}

void Mesh::setDiffuseTexture(const std::filesystem::path& pathToImageFile) {
    // Delete previous texture.
    glDeleteTextures(1, &iDiffuseTextureId);

    // Create new texture.
    iDiffuseTextureId = Application::loadTexture(pathToImageFile);
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
