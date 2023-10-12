#include "TextureImporter.h"

// Standard.
#include <format>
#include <array>

// Custom.
#include "window/GLFW.hpp"

// External.
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

bool TextureImporter::bFlipTexturesVertically = false;

unsigned int TextureImporter::loadTexture(const std::filesystem::path& pathToImage, bool bIsDiffuseTexture) {
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
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        bIsDiffuseTexture
            ? GL_SRGB    // Specifying `SRGB` so that OpenGL will correct the colors
            : iGlFormat, // to linear-space as soon as we use them to avoid applying gamma correction twice.
        iWidth,
        iHeight,
        0,
        iGlFormat,
        GL_UNSIGNED_BYTE,
        pPixels);

    // Generate mipmaps.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free pixels.
    stbi_image_free(pPixels);

    return iTextureId;
}

unsigned int TextureImporter::loadCubemap(const std::filesystem::path& pathToImagesDirectory) {
    // Make sure the specified path exists.
    if (!std::filesystem::exists(pathToImagesDirectory)) [[unlikely]] {
        throw std::runtime_error(
            std::format("the specified path \"{}\" does not exists", pathToImagesDirectory.string()));
    }

    // Make sure the specified path points to a directory.
    if (!std::filesystem::is_directory(pathToImagesDirectory)) [[unlikely]] {
        throw std::runtime_error(std::format(
            "expected the specified path \"{}\" to be a directory", pathToImagesDirectory.string()));
    }

    // Create a new cubemap object.
    unsigned int iCubemapId = 0;
    glGenTextures(1, &iCubemapId);

    // Bind texture to texture target to update its data.
    glBindTexture(GL_TEXTURE_CUBE_MAP, iCubemapId);

    // Prepare image format.
    const auto iStbiFormat = STBI_rgb;
    const auto iGlFormat = GL_RGB;

    // Prepare image file names.
    std::array<std::string, 6> vFilenames = {// NOLINT: magic number - 6 faces
                                             "right.jpg",
                                             "left.jpg",
                                             "top.jpg",
                                             "bottom.jpg",
                                             "front.jpg",
                                             "back.jpg"};

    for (size_t i = 0; i < vFilenames.size(); i++) {
        // Prepare to load image pixels.
        int iWidth = 0;
        int iHeight = 0;
        int iChannels = 0;
        const auto pathToImage = pathToImagesDirectory / vFilenames[i];

        // Load image pixels.
        const auto pPixels =
            stbi_load(pathToImage.string().c_str(), &iWidth, &iHeight, &iChannels, iStbiFormat);
        if (pPixels == nullptr) [[unlikely]] {
            throw std::runtime_error(
                std::format("failed to load image from path \"{}\"", pathToImage.string()));
        }

        // Load pixels to one face of the cubemap.
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int>(i),
            0,
            GL_SRGB,
            iWidth,
            iHeight,
            0,
            iGlFormat,
            GL_UNSIGNED_BYTE,
            pPixels);

        // Free pixels.
        stbi_image_free(pPixels);
    }

    // Set cubemap texture filtering.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // no mipmaps on cubemap
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR); // no need to set `MIPMAP` option since magnification does not use mipmaps

    // Set cubemap texture wrapping
    // (return edge values for out of bounds `sample` calls).
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return iCubemapId;
}
