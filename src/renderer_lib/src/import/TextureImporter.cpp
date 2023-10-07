#include "TextureImporter.h"

// Standard.
#include <format>

// Custom.
#include "window/GLFW.hpp"

// External.
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

bool TextureImporter::bFlipTexturesVertically = false;

unsigned int TextureImporter::loadTexture(const std::filesystem::path& pathToImage) {
    // Make sure the specified path exists.
    if (!std::filesystem::exists(pathToImage)) [[unlikely]] {
        throw std::runtime_error(
            std::format("the specified path \"{}\" does not exists", pathToImage.string()));
    }

    // Prepare image format.
    const auto iStbiFormat = STBI_rgb;
    const auto iGlFormat = GL_RGB;

    // Flip images vertically when loading.
    stbi_set_flip_vertically_on_load(static_cast<int>(bFlipTexturesVertically));

    // Load image pixels.
    int iWidth = 0;
    int iHeight = 0;
    int iChannels = 0;
    const auto pPixels = stbi_load(pathToImage.string().c_str(), &iWidth, &iHeight, &iChannels, iStbiFormat);
    if (pPixels == nullptr) [[unlikely]] {
        throw std::runtime_error(std::format("failed to load image from path \"{}\"", pathToImage.string()));
    }

    // Create a new texture object.
    unsigned int iTextureId = 0;
    glGenTextures(1, &iTextureId);

    // Bind texture to texture target to update its data.
    glBindTexture(GL_TEXTURE_2D, iTextureId);

    // Copy pixels to the texture.
    glTexImage2D(GL_TEXTURE_2D, 0, iGlFormat, iWidth, iHeight, 0, iGlFormat, GL_UNSIGNED_BYTE, pPixels);

    // Generate mipmaps.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free pixels.
    stbi_image_free(pPixels);

    return iTextureId;
}
