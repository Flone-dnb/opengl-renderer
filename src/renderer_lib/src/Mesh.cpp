#include "Mesh.h"

// Custom.
#include "GLFW.hpp"
#include "Application.h"

void Vertex::setVertexAttributes() {
    // Prepare offsets of fields.
    const auto iPositionOffset = offsetof(Vertex, position);
    const auto iUvOffset = offsetof(Vertex, uv);

    // Specify position.
    glVertexAttribPointer(
        0,                 // attribute index (layout location)
        3,                 // number of components
        GL_FLOAT,          // type of component
        GL_FALSE,          // whether data should be normalized or not
        sizeof(Vertex),    // stride (size in bytes between elements)
        &iPositionOffset); // beginning offset
    glEnableVertexAttribArray(0);

    // Specify UV.
    glVertexAttribPointer(
        1,              // attribute index (layout location)
        2,              // number of components
        GL_FLOAT,       // type of component
        GL_FALSE,       // whether data should be normalized or not
        sizeof(Vertex), // stride (size in bytes between elements)
        &iUvOffset);    // beginning offset
    glEnableVertexAttribArray(1);
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
}

std::unique_ptr<Mesh> Mesh::create(std::vector<Vertex>&& vVertices, std::vector<unsigned int>&& vIndices) {
    static_assert(sizeof(vIndices[0]) == sizeof(unsigned int), "change index format in the `draw` command");

    // Prepare the resulting mesh.
    auto pMesh = std::make_unique<Mesh>();

    // Prepare vertex/index buffers.
    pMesh->prepareVertexBuffer(std::move(vVertices));
    pMesh->prepareIndexBuffer(std::move(vIndices));

    // Describe vertex attributes.
    Vertex::setVertexAttributes();

    return pMesh;
}

void Mesh::prepareVertexBuffer(std::vector<Vertex>&& vVertices) {
    // Create vertex buffer object (VBO).
    glGenBuffers(1, &iVertexBufferObjectId);

    // Create vertex array object (VAO).
    glGenVertexArrays(1, &iVertexArrayObjectId);

    // Bind our vertex array object to the OpenGL context.
    glBindVertexArray(iVertexArrayObjectId);

    // Set our vertex buffer to the "array" target  in OpenGL context to update its data.
    glBindBuffer(GL_ARRAY_BUFFER, iVertexBufferObjectId);

    // Copy vertices to the buffer.
    glBufferData(
        GL_ARRAY_BUFFER,
        vVertices.size() * sizeof(vVertices[0]),
        vVertices.data(),
        GL_STATIC_DRAW); // `STATIC` because the data will not be changed
}

void Mesh::prepareIndexBuffer(std::vector<unsigned int>&& vIndices) {
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

    // Save the total number of indices.
    iIndexCount = vIndices.size();
}
