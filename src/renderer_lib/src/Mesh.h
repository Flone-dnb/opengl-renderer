#pragma once

// Standard.
#include <vector>
#include <memory>

// Custom.
#include "GLMath.hpp"

/** Groups information to draw an object. */
struct Mesh {
    ~Mesh();

    /**
     * Creates a new mesh with the specified data.
     *
     * @remark Expects that OpenGL is initialized.
     *
     * @param vVertices Vertices of the mesh.
     * @param vIndices  Indices of the mesh.
     *
     * @return Created mesh. The resulting object is wrapped into a `unique_ptr` for "move" simplicity
     * (so that I don't need to implement move functions and make sure created buffers will not be
     * deleted multiple times).
     */
    static std::unique_ptr<Mesh>
    create(std::vector<glm::vec3>&& vVertices, std::vector<unsigned int>&& vIndices);

    /** Total number of indices that this mesh has. */
    unsigned int iIndexCount = 0;

    /** ID of the vertex buffer object. */
    unsigned int iVertexBufferObjectId = 0;

    /** ID of the index buffer object. */
    unsigned int iIndexBufferObjectId = 0;

    /** ID of the vertex array object that references a vertex buffer object and its attributes. */
    unsigned int iVertexArrayObjectId = 0;

private:
    /** Describes to OpenGL how vertex data should be interpreted. */
    static void setVertexAttributes();

    /**
     * Creates a vertex buffer, fills it and assigns it to the OpenGL context. The resulting buffer object ID
     * is assigned to @ref iVertexBufferObjectId.
     *
     * @param vVertices Vertices to use.
     */
    void prepareVertexBuffer(std::vector<glm::vec3>&& vVertices);

    /**
     * Creates an index buffer, fills it and assigns it to the OpenGL context. The resulting buffer object ID
     * is assigned to @ref iIndexBufferObjectId.
     *
     * @param vIndices Indices to use.
     */
    void prepareIndexBuffer(std::vector<unsigned int>&& vIndices);
};
