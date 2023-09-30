#include "Application.h"

// Standard.
#include <format>
#include <iostream>
#include <fstream>

// Custom.
#include "GLFW.hpp"
#include "ShaderIncluder.h"

// External.
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void GLAPIENTRY opengGlMessageCallback(
    GLenum source,
    GLenum type,
    GLuint id, // NOLINT
    GLenum severity,
    GLsizei length,
    const GLchar* message,   // NOLINT
    const void* userParam) { // NOLINT
    fprintf(
        stderr,
        "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type,
        severity,
        message);
}

unsigned int Application::loadTexture(const std::filesystem::path& pathToImage) {
    // Make sure the specified path exists.
    if (!std::filesystem::exists(pathToImage)) [[unlikely]] {
        throw std::runtime_error(
            std::format("the specified path \"{}\" does not exists", pathToImage.string()));
    }

    // Prepare image format.
    const auto iStbiFormat = STBI_rgb;
    const auto iGlFormat = GL_RGB;

    // Load image pixels.
    int iWidth = 0;
    int iHeight = 0;
    int iChannels = 0;
    const auto pPixels = stbi_load(pathToImage.c_str(), &iWidth, &iHeight, &iChannels, iStbiFormat);
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

void Application::run() {
    initWindow();
    initOpenGl();
    prepareScene();
    mainLoop();
}

void Application::initWindow() {
    // Initialize GLFW.
    GLFW::get();

    // Create GLFW window.
    constexpr int iWindowWidth = 800;  // NOLINT
    constexpr int iWindowHeight = 600; // NOLINT
    pGLFWWindow =
        glfwCreateWindow(iWindowWidth, iWindowHeight, "OpenGL", nullptr, nullptr); // NOLINT: magic number
    if (pGLFWWindow == nullptr) {
        throw std::runtime_error("failed to create window");
    }

    // Make window's OpenGL context to be the current one for this thread.
    glfwMakeContextCurrent(pGLFWWindow);

    // Initialize GLAD by passing it a function that will be used to load OpenGL function addresses
    // (GLFW provides a cross-platform function to load OpenGL functions).
    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
        throw std::runtime_error("failed to initialize GLAD");
    }

    // Specify the initial window size to OpenGL.
    glViewport(0, 0, iWindowWidth, iWindowHeight);

    // Bind to framebuffer size change event.
    glfwSetFramebufferSizeCallback(pGLFWWindow, Application::glfwFramebufferResizeCallback);

    // Bind to keyboard input.
    glfwSetKeyCallback(pGLFWWindow, Application::glfwWindowKeyboardCallback);
}

void Application::initOpenGl() {
#if defined(DEBUG)
    // Enable debug output.
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengGlMessageCallback, 0);
#endif

    // Specify clear color.
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    // Set shaders to context.
    prepareShaders();

    // Set texture wrapping.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set texture filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR); // no need to set `MIPMAP` option since magnification does not use mipmaps
}

void Application::mainLoop() {
    while (glfwWindowShouldClose(pGLFWWindow) == 0) {
        // Process window events.
        glfwPollEvents();

        drawNextFrame();

        // Swap back/front buffers.
        glfwSwapBuffers(pGLFWWindow);
    }
}

void Application::prepareScene() {
    // NOLINTBEGIN(readability-magic-numbers)

    vMeshesToDraw.push_back(Mesh::create(
        {{glm::vec3(0.5F, 0.5F, 0.0F), glm::vec2(1.0F, 1.0F)},
         {glm::vec3(0.5F, -0.5F, 0.0F), glm::vec2(1.0F, 0.0F)},
         {glm::vec3(-0.5F, -0.5F, 0.0F), glm::vec2(0.0F, 0.0F)},
         {glm::vec3(-0.5F, 0.5F, 0.0F), glm::vec2(0.0F, 1.0F)}},
        {0, 1, 3, 1, 2, 3}));

    // NOLINTEND(readability-magic-numbers)
}

void Application::drawNextFrame() const {
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shaders.
    glUseProgram(iShaderProgramId);

    for (const auto& mesh : vMeshesToDraw) {
        // Set vertex array object.
        glBindVertexArray(mesh->iVertexArrayObjectId);

        // Set diffuse texture.
        glBindTexture(GL_TEXTURE_2D, mesh->iDiffuseTextureId);

        // Submit a draw command.
        glDrawElements(GL_TRIANGLES, mesh->iIndexCount, GL_UNSIGNED_INT, 0);
    }
}

void Application::prepareShaders() {
    // Prepare shaders.
    const auto iVertexShaderId = compileShader("res/shaders/vertex.glsl", true);
    const auto iFragmentShaderId = compileShader("res/shaders/fragment.glsl", false);

    // Create shader program.
    iShaderProgramId = glCreateProgram();

    // Attach shaders to shader program.
    glAttachShader(iShaderProgramId, iVertexShaderId);
    glAttachShader(iShaderProgramId, iFragmentShaderId);

    // Link shaders together.
    glLinkProgram(iShaderProgramId);

    // See if there were any linking errors.
    int iSuccess = 0;
    std::array<char, 1024> infoLog = {0}; // NOLINT
    glGetProgramiv(iShaderProgramId, GL_LINK_STATUS, &iSuccess);
    if (iSuccess == 0) {
        glGetProgramInfoLog(iShaderProgramId, infoLog.size(), NULL, infoLog.data());
        throw std::runtime_error(std::format("failed to link shader program, error: {}", infoLog.data()));
    }

    // Delete shaders since we don't need them anymore.
    glDeleteShader(iVertexShaderId);
    glDeleteShader(iFragmentShaderId);
}

void Application::glfwFramebufferResizeCallback(GLFWwindow* pGlfwWindow, int iWidth, int iHeight) {
    // Update viewport size according to the new framebuffer size.
    glViewport(0, 0, iWidth, iHeight);
}

void Application::glfwWindowKeyboardCallback(
    GLFWwindow* pGlfwWindow, int iKey, int iScancode, int iAction, int iMods) {
    // Ignore "repeat" actions.
    if (iAction == GLFW_REPEAT) {
        return;
    }

    // See if we need to close the window.
    if (iKey == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(pGlfwWindow, 1);
    }
}

unsigned int
Application::compileShader(const std::filesystem::__cxx11::path& pathToShader, bool bIsVertexShader) {
    // Make sure the specified path exists.
    if (!std::filesystem::exists(pathToShader)) [[unlikely]] {
        throw std::runtime_error(std::format("expected the path {} to exist", pathToShader.string()));
    }

    // Load shader code from disk.
    auto result = ShaderIncluder::parseFullSourceCode(pathToShader);
    if (std::holds_alternative<ShaderIncluder::Error>(result)) {
        throw std::runtime_error(std::format(
            "failed to parse shader source code, error: {}",
            static_cast<int>(std::get<ShaderIncluder::Error>(result))));
    }
    const auto sFullSourceCode = std::get<std::string>(std::move(result));

    // Create shader.
    const auto iShaderId = glCreateShader(bIsVertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

    // Attach shader source code to our created shader.
    std::array<const char*, 1> vCodesToAttach = {sFullSourceCode.c_str()};
    glShaderSource(iShaderId, vCodesToAttach.size(), vCodesToAttach.data(), NULL);

    // Compile shader.
    glCompileShader(iShaderId);

    // See if there were any warnings/errors.
    int iSuccess = 0;
    std::array<char, 1024> infoLog = {0}; // NOLINT
    glGetShaderiv(iShaderId, GL_COMPILE_STATUS, &iSuccess);
    if (iSuccess == 0) {
        glGetShaderInfoLog(iShaderId, infoLog.size(), NULL, infoLog.data());
        throw std::runtime_error(std::format(
            "failed to compile shader from {}, error: {}", pathToShader.string(), infoLog.data()));
    }

    return iShaderId;
}
