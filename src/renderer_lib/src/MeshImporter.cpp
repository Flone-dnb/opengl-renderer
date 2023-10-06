#include "MeshImporter.h"

// Standard.
#include <format>

// External.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

inline bool writeGltfTextureToDisk(const tinygltf::Image& image, const std::filesystem::path& pathToImage) {
    // Prepare callbacks.
    tinygltf::FsCallbacks fsCallbacks = {
        &tinygltf::FileExists,
        &tinygltf::ExpandFilePath,
        &tinygltf::ReadWholeFile,
        &tinygltf::WriteWholeFile,
        &tinygltf::GetFileSizeInBytes,
        nullptr};
    tinygltf::URICallbacks uriCallbacks;
    uriCallbacks.encode = nullptr;
    uriCallbacks.decode = [](const std::string& sInUri, std::string* pOutUri, void* pUserData) -> bool {
        *pOutUri = sInUri;
        return true;
    };

    // Prepare paths.
    const auto sFilename = pathToImage.stem().string() + pathToImage.extension().string();
    const auto sBasePath = pathToImage.parent_path().string();
    std::string sOutputUri;

    // Write image to disk.
    return tinygltf::WriteImageData(
        &sBasePath, &sFilename, &image, false, &uriCallbacks, &sOutputUri, &fsCallbacks);
}

inline void processGltfMesh( // NOLINT: too complex
    const tinygltf::Model& model,
    const tinygltf::Mesh& mesh,
    std::vector<std::unique_ptr<Mesh>>& vImportedMeshes,
    const std::filesystem::path& pathToFile) {
    // Prepare variables.
    const std::string sImageExtension = ".png";
    const std::string sDiffuseTextureName = "diffuse";

    // Prepare paths.
    const std::filesystem::path pathToTempFiles = pathToFile.parent_path() / "temp";
    if (std::filesystem::exists(pathToTempFiles)) {
        std::filesystem::remove_all(pathToTempFiles);
    }
    std::filesystem::create_directory(pathToTempFiles);

    // Go through each mesh in this node.
    for (size_t iPrimitive = 0; iPrimitive < mesh.primitives.size(); iPrimitive++) {
        auto& primitive = mesh.primitives[iPrimitive];

        // Prepare a new mesh data.
        std::vector<Vertex> vVertices;
        std::vector<unsigned int> vIndices;

        {
            // Add indices.
            // Save accessor to mesh indices.
            const auto& indexAccessor = model.accessors[primitive.indices];

            // Get index buffer.
            const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const auto& indexBuffer = model.buffers[indexBufferView.buffer];

            // Make sure index is stored as scalar`.
            if (indexAccessor.type != TINYGLTF_TYPE_SCALAR) [[unlikely]] {
                throw std::runtime_error(std::format(
                    "expected indices of mesh to be stored as `scalar`, actual type: {}",
                    indexAccessor.type));
            }

            // Prepare variables to read indices.
            auto pCurrentIndex =
                indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
            vIndices.resize(indexAccessor.count);

            // Allocate indices depending on their type.
            if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                using index_t = unsigned int;

                const auto iStride =
                    indexBufferView.byteStride == 0 ? sizeof(index_t) : indexBufferView.byteStride;

                // Set indices.
                for (size_t i = 0; i < vIndices.size(); i++) {
                    // Set value.
                    vIndices[i] = reinterpret_cast<const index_t*>(pCurrentIndex)[0];

                    // Switch to the next item.
                    pCurrentIndex += iStride;
                }
            } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                using index_t = unsigned short;

                const auto iStride =
                    indexBufferView.byteStride == 0 ? sizeof(index_t) : indexBufferView.byteStride;

                // Set indices.
                for (size_t i = 0; i < vIndices.size(); i++) {
                    // Set value.
                    vIndices[i] = reinterpret_cast<const index_t*>(pCurrentIndex)[0];

                    // Switch to the next item.
                    pCurrentIndex += iStride;
                }
            } else {
                throw std::runtime_error(std::format(
                    "expected indices mesh component type to be `unsigned int` or `unsigned short`, "
                    "actual type: {}",
                    indexAccessor.componentType));
            }
        }

        {
            // Find a position attribute to know how much vertices there will be.
            const auto it = primitive.attributes.find("POSITION");
            if (it == primitive.attributes.end()) [[unlikely]] {
                throw std::runtime_error("a GLTF mesh node does not have any positions defined");
            }
            const auto iPositionAccessorIndex = it->second;

            // Get accessor.
            const auto& positionAccessor = model.accessors[iPositionAccessorIndex];

            // Allocate vertices.
            vVertices.resize(positionAccessor.count);
        }

        // Process attributes.
        size_t iProcessedAttributeCount = 0;
        for (auto& [sAttributeName, iAccessorIndex] : primitive.attributes) {
            // Get attribute accessor.
            const auto& attributeAccessor = model.accessors[iAccessorIndex];

            // Get buffer.
            const auto& attributeBufferView = model.bufferViews[attributeAccessor.bufferView];
            const auto& attributeBuffer = model.buffers[attributeBufferView.buffer];

            if (sAttributeName == "POSITION") {
                using position_t = glm::vec3;

                // Make sure position is stored as `vec3`.
                if (attributeAccessor.type != TINYGLTF_TYPE_VEC3) [[unlikely]] {
                    throw std::runtime_error(std::format(
                        "expected POSITION mesh attribute to be stored as `vec3`, actual type: {}",
                        attributeAccessor.type));
                }
                // Make sure that component type is `float`.
                if (attributeAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) [[unlikely]] {
                    throw std::runtime_error(std::format(
                        "expected POSITION mesh attribute component type to be `float`, actual type: {}",
                        attributeAccessor.componentType));
                }

                // Prepare variables.
                auto pCurrentPosition = attributeBuffer.data.data() + attributeBufferView.byteOffset +
                                        attributeAccessor.byteOffset;
                const auto iStride =
                    attributeBufferView.byteStride == 0 ? sizeof(position_t) : attributeBufferView.byteStride;

                // Set positions to mesh data.
                for (size_t i = 0; i < vVertices.size(); i++) {
                    // Set value.
                    vVertices[i].position = reinterpret_cast<const position_t*>(pCurrentPosition)[0];

                    // Switch to the next item.
                    pCurrentPosition += iStride;
                }

                // Process next attribute.
                continue;
            }

            if (sAttributeName == "NORMAL") {
                using normal_t = glm::vec3;

                // Make sure normal is stored as `vec3`.
                if (attributeAccessor.type != TINYGLTF_TYPE_VEC3) [[unlikely]] {
                    throw std::runtime_error(std::format(
                        "expected NORMAL mesh attribute to be stored as `vec3`, actual type: {}",
                        attributeAccessor.type));
                }
                // Make sure that component type is `float`.
                if (attributeAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) [[unlikely]] {
                    throw std::runtime_error(std::format(
                        "expected NORMAL mesh attribute component type to be `float`, actual type: {}",
                        attributeAccessor.componentType));
                }

                // Prepare variables.
                auto pCurrentNormal = attributeBuffer.data.data() + attributeBufferView.byteOffset +
                                      attributeAccessor.byteOffset;
                const auto iStride =
                    attributeBufferView.byteStride == 0 ? sizeof(normal_t) : attributeBufferView.byteStride;

                // Set normals to mesh data.
                for (size_t i = 0; i < vVertices.size(); i++) {
                    // Set value.
                    vVertices[i].normal = reinterpret_cast<const normal_t*>(pCurrentNormal)[0];

                    // Switch to the next item.
                    pCurrentNormal += iStride;
                }

                // Process next attribute.
                continue;
            }

            if (sAttributeName == "TEXCOORD_0") {
                using uv_t = glm::vec2;

                // Make sure UV is stored as `vec2`.
                if (attributeAccessor.type != TINYGLTF_TYPE_VEC2) [[unlikely]] {
                    throw std::runtime_error(std::format(
                        "expected TEXCOORD mesh attribute to be stored as `vec2`, actual type: {}",
                        attributeAccessor.type));
                }
                // Make sure that component type is `float`.
                if (attributeAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) [[unlikely]] {
                    throw std::runtime_error(std::format(
                        "expected TEXCOORD mesh attribute component type to be `float`, actual type: {}",
                        attributeAccessor.componentType));
                }

                // Prepare variables.
                auto pCurrentUv = attributeBuffer.data.data() + attributeBufferView.byteOffset +
                                  attributeAccessor.byteOffset;
                const auto iStride =
                    attributeBufferView.byteStride == 0 ? sizeof(uv_t) : attributeBufferView.byteStride;

                // Set UVs to mesh data.
                for (size_t i = 0; i < vVertices.size(); i++) {
                    // Set value.
                    vVertices[i].uv = reinterpret_cast<const uv_t*>(pCurrentUv)[0];

                    // Switch to the next item.
                    pCurrentUv += iStride;
                }

                // Process next attribute.
                continue;
            }
        }

        // Create a new mesh node with the specified data.
        auto pNewMesh = Mesh::create(std::move(vVertices), std::move(vIndices));

        if (primitive.material >= 0) {
            // Process material.
            auto& material = model.materials[primitive.material];

            // Process diffuse texture.
            const auto iDiffuseTextureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
            if (iDiffuseTextureIndex >= 0) {
                auto& diffuseTexture = model.textures[iDiffuseTextureIndex];
                if (diffuseTexture.source >= 0) {
                    // Get image.
                    auto& diffuseImage = model.images[diffuseTexture.source];

                    // Prepare path to export the image to.
                    const auto pathToDiffuseImage = pathToTempFiles / (sDiffuseTextureName + sImageExtension);

                    // Write image to disk.
                    if (!writeGltfTextureToDisk(diffuseImage, pathToDiffuseImage)) {
                        throw std::runtime_error(std::format(
                            "failed to write GLTF image to path \"{}\"", pathToDiffuseImage.string()));
                    }

                    // Load texture.
                    pNewMesh->setDiffuseTexture(pathToDiffuseImage);
                }
            }
        }

        // Add this new mesh node to results.
        vImportedMeshes.push_back(std::move(pNewMesh));
    }

    // Cleanup.
    std::filesystem::remove_all(pathToTempFiles);
}

inline void processGltfNode(
    const tinygltf::Node& node,
    const tinygltf::Model& model,
    std::vector<std::unique_ptr<Mesh>>& vImportedMeshes,
    const std::filesystem::path& pathToFile) {
    // See if this node stores a mesh.
    if ((node.mesh >= 0) && (static_cast<size_t>(node.mesh) < model.meshes.size())) {
        // Process mesh.
        processGltfMesh(model, model.meshes[node.mesh], vImportedMeshes, pathToFile);
    }

    // Process child nodes.
    for (const auto& iNode : node.children) {
        processGltfNode(model.nodes[iNode], model, vImportedMeshes, pathToFile);
    }
}

std::vector<std::unique_ptr<Mesh>> MeshImporter::importMesh(const std::filesystem::path& pathToFile) {
    // Make sure the file has ".GLTF" or ".GLB" extension.
    if (pathToFile.extension() != ".GLTF" && pathToFile.extension() != ".gltf" &&
        pathToFile.extension() != ".GLB" && pathToFile.extension() != ".glb") [[unlikely]] {
        throw std::runtime_error(std::format(
            "only GLTF/GLB file extension is supported for mesh import, the path \"{}\" points to a "
            "non-GLTF file",
            pathToFile.string()));
    }

    // Make sure the specified path to the file exists.
    if (!std::filesystem::exists(pathToFile)) [[unlikely]] {
        throw std::runtime_error(
            std::format("the specified path \"{}\" does not exists", pathToFile.string()));
    }

    // See if we have a binary GTLF file or not.
    bool bIsGlb = false;
    if (pathToFile.extension() == ".GLB" || pathToFile.extension() == ".glb") {
        bIsGlb = true;
    }

    using namespace tinygltf;

    // Prepare variables for storing results.
    Model model;
    TinyGLTF loader;
    std::string sError;
    std::string sWarning;
    bool bIsSuccess = false;

    // Don't make all images to be in RGBA format.
    loader.SetPreserveImageChannels(true);

    // Load data from file.
    if (bIsGlb) {
        bIsSuccess = loader.LoadBinaryFromFile(&model, &sError, &sWarning, pathToFile.string());
    } else {
        bIsSuccess = loader.LoadASCIIFromFile(&model, &sError, &sWarning, pathToFile.string());
    }

    // See if there were any warnings/errors.
    if (!sWarning.empty()) {
        // Treat warnings as errors.
        throw std::runtime_error(std::format("there was an error during the import process: {}", sWarning));
    }
    if (!sError.empty()) {
        throw std::runtime_error(std::format("there was an error during the import process: {}", sError));
    }
    if (!bIsSuccess) {
        throw std::runtime_error(
            "there was an error during the import process but no error message was received");
    }

    // Prepare variable for processed nodes.
    size_t iTotalNodeProcessedCount = 0;

    // Get default scene.
    const auto& scene = model.scenes[model.defaultScene];

    std::vector<std::unique_ptr<::Mesh>> vImportedMeshes;

    for (const auto& iNode : scene.nodes) {
        // Make sure this node index is valid.
        if (iNode < 0) [[unlikely]] {
            throw std::runtime_error(
                std::format("found a negative node index of {} in default scene", iNode));
        }
        if (static_cast<size_t>(iNode) >= model.nodes.size()) [[unlikely]] {
            throw std::runtime_error(std::format(
                "found an out of bounds node index of {} while model nodes only has {} entries",
                iNode,
                model.nodes.size()));
        }

        // Process node.
        processGltfNode(model.nodes[iNode], model, vImportedMeshes, pathToFile);
    }

    return vImportedMeshes;
}
