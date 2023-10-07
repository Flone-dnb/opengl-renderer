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

    /** Whether we need to flip the texture vertically during the import or not. */
    static bool bFlipTexturesVertically;
};
