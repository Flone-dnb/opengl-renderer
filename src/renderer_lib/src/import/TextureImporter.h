#pragma once

// Standard.
#include <filesystem>

/** Provides static functions for importing (loading) textures. */
class TextureImporter {
public:
    TextureImporter() = delete;

    /**
     * Loads the specified image and returns its ID.
     *
     * @remark Expects that OpenGL is initialized.
     *
     * @param pathToImage
     *
     * @return Loaded image ID.
     */
    static unsigned int loadTexture(const std::filesystem::path& pathToImage);

    /**
     * Looks into the specified directory with 6 textures named "back", "right", "front", "left", "top",
     * "bottom" and loads them as one cubemap.
     *
     * @param pathToImagesDirectory Path to the directory with images.
     *
     * @return ID of the loaded cubemap.
     */
    static unsigned int loadCubemap(const std::filesystem::path& pathToImagesDirectory);

    /** Whether we need to flip the texture vertically during the import or not. */
    static bool bFlipTexturesVertically;
};
