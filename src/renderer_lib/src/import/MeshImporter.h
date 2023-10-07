#pragma once

// Standard.
#include <filesystem>
#include <optional>
#include <functional>

// Custom.
#include "Mesh.h"

/**
 * Provides static functions for importing files in special formats (such as GLTF/GLB) as meshes,
 * textures, etc.
 */
class MeshImporter {
public:
    MeshImporter() = delete;

    /**
     * Imports a file in a special format (such as GTLF/GLB).
     *
     * @param pathToFile Path to the file to import.
     *
     * @return Imported meshes.
     */
    static std::vector<std::unique_ptr<Mesh>> importMesh(const std::filesystem::path& pathToFile);
};
