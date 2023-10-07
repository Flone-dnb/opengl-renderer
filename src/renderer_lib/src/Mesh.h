#pragma once

// Standard.
#include <vector>
#include <filesystem>
#include <memory>

// Custom.
#include "math/GLMath.hpp"
#include "shapes/AABB.h"

/** Determines material properties of a mesh. */
struct Material {
    /**
     * Sets material's properties to the specified shader.
     *
     * @param iShaderProgramId Shader program to set material's properties to.
     */
    void setToShader(unsigned int iShaderProgramId) const;

    /** ID of the diffuse texture (if used). */
    unsigned int iDiffuseTextureId = 0;

    /** Diffuse light color. */
    glm::vec3 diffuseColor = glm::vec3(1.0F, 1.0F, 1.0F);

    /** Specular light color. */
    glm::vec3 specularColor = glm::vec3(1.0F, 1.0F, 1.0F);

    /** Determines how shiny the surface is. */
    float shininess = 32.0F; // NOLINT
};

/** Groups information about one vertex. */
struct Vertex {
    /** Describes to OpenGL how vertex data should be interpreted. */
    static void setVertexAttributes();

    /** Vertex position in model space. */
    glm::vec3 position;

    /** Vertex normal vector in model space. */
    glm::vec3 normal;

    /** UV coordinate. */
    glm::vec2 uv;
};

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
    create(std::vector<Vertex>&& vVertices, std::vector<unsigned int>&& vIndices);

    /**
     * Assigns the specified diffuse texture to be used.
     *
     * @param pathToImageFile Path to the image file to use.
     */
    void setDiffuseTexture(const std::filesystem::path& pathToImageFile);

    /**
     * Sets new world matrix to be used.
     *
     * @param newWorldMatrix World matrix.
     */
    void setWorldMatrix(const glm::mat4x4& newWorldMatrix);

    /**
     * Returns matrix that transforms data (such as positions) from model space to world space.
     *
     * @return World matrix.
     */
    glm::mat4x4* getWorldMatrix();

    /**
     * Returns matrix that transforms normals from model space to world space.
     *
     * @return Normal matrix.
     */
    glm::mat3x3* getNormalMatrix();

    /** Mesh's material. */
    Material material;

    /** Mesh's AABB in model space. */
    AABB aabb;

    /** ID of the vertex array object that references a vertex buffer object and its attributes. */
    unsigned int iVertexArrayObjectId = 0;

    /** ID of the index buffer object. */
    unsigned int iIndexBufferObjectId = 0;

    /** Total number of indices in the mesh. */
    int iIndexCount = 0;

private:
    /**
     * Calculates a normal matrix from a world matrix.
     *
     * @param worldMatrix World matrix.
     *
     * @return Normal matrix.
     */
    static glm::mat3x3 getNormalMatrixFromWorldMatrix(const glm::mat4x4& worldMatrix);

    /**
     * Creates a vertex buffer, fills it and assigns it to the OpenGL context. The resulting buffer object ID
     * is assigned to @ref iVertexBufferObjectId.
     *
     * @param vVertices Vertices to use.
     */
    void prepareVertexBuffer(std::vector<Vertex>&& vVertices);

    /**
     * Creates an index buffer, fills it and assigns it to the OpenGL context. The resulting buffer object ID
     * is assigned to @ref iIndexBufferObjectId.
     *
     * @param vIndices Indices to use.
     */
    void prepareIndexBuffer(std::vector<unsigned int>&& vIndices);

    /** Matrix that transforms data (such as positions) from model space to world space. */
    glm::mat4x4 worldMatrix = glm::identity<glm::mat4x4>();

    /** Matrix that uniformly transform normals from model space to world space. */
    glm::mat3x3 normalMatrix = glm::identity<glm::mat3x3>();

    /** ID of the vertex buffer object. */
    unsigned int iVertexBufferObjectId = 0;
};
