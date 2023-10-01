#include "Application.h"

// Standard.
#include <format>
#include <iostream>
#include <fstream>

// Custom.
#include "GLFW.hpp"
#include "ShaderIncluder.h"
#include "MeshImporter.h"

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
    if (type == GL_DEBUG_TYPE_OTHER) {
        return; // ignore "other" messages
    }

    using namespace std;
    cout << "---------------------opengl-callback-start------------" << endl;
    cout << "message: " << message << endl;
    cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        cout << "OTHER";
        break;
    }
    cout << endl;

    cout << "id: " << id << endl;
    cout << "severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        cout << "HIGH";
        break;
    }
    cout << endl;
    cout << "---------------------opengl-callback-end--------------" << endl;
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

    // Flip images vertically when loading.
    stbi_set_flip_vertically_on_load(1);

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

    // Add Application pointer.
    glfwSetWindowUserPointer(pGLFWWindow, this);

    // Make window's OpenGL context to be the current one for this thread.
    glfwMakeContextCurrent(pGLFWWindow);

    // Initialize GLAD by passing it a function that will be used to load OpenGL function addresses
    // (GLFW provides a cross-platform function to load OpenGL functions).
    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
        throw std::runtime_error("failed to initialize GLAD");
    }

    // Specify the initial window size to OpenGL.
    glViewport(0, 0, iWindowWidth, iWindowHeight);

    // Update projection matrix.
    projectionMatrix = glm::perspective(
        glm::radians(verticalFov),
        static_cast<float>(iWindowWidth) / iWindowHeight,
        nearClipPlaneZ,
        farClipPlaneZ);

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

    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);

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
    // Import meshes from file.
    auto vImportedMeshes = MeshImporter::importMesh("res/mesh.glb");
    for (auto& pMesh : vImportedMeshes) {
        vMeshesToDraw.push_back(std::move(pMesh));
    }

    // Setup camera.
    viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0F, 0.0F, -5.0F)); // NOLINT
}

void Application::drawNextFrame() const {
    // Clear color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set shaders.
    glUseProgram(iShaderProgramId);

    // Set view/projection matrix.
    const auto iViewProjectionMatrixLocation = glGetUniformLocation(iShaderProgramId, "viewProjectionMatrix");
    if (iViewProjectionMatrixLocation < 0) [[unlikely]] {
        throw std::runtime_error("unable to get location for view/projection matrix");
    }
    const auto viewProjectionMatrix = projectionMatrix * viewMatrix;
    glUniformMatrix4fv(iViewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

    for (const auto& mesh : vMeshesToDraw) {
        // Set world matrix.
        const auto iWorldMatrixLocation = glGetUniformLocation(iShaderProgramId, "worldMatrix");
        if (iWorldMatrixLocation < 0) [[unlikely]] {
            throw std::runtime_error("unable to get location for world matrix");
        }
        glUniformMatrix4fv(iWorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(mesh->worldMatrix));

        // Set vertex array object.
        glBindVertexArray(mesh->iVertexArrayObjectId);

        // Set element object.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->iIndexBufferObjectId);

        // Set diffuse texture at texture unit (location) 0.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->iDiffuseTextureId);

        // Submit a draw command.
        glDrawElements(GL_TRIANGLES, mesh->iIndexCount, GL_UNSIGNED_INT, nullptr);
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
        glGetProgramInfoLog(iShaderProgramId, static_cast<int>(infoLog.size()), NULL, infoLog.data());
        throw std::runtime_error(std::format("failed to link shader program, error: {}", infoLog.data()));
    }

    // Delete shaders since we don't need them anymore.
    glDeleteShader(iVertexShaderId);
    glDeleteShader(iFragmentShaderId);
}

void Application::glfwFramebufferResizeCallback(GLFWwindow* pGlfwWindow, int iWidth, int iHeight) {
    const auto pApplication = static_cast<Application*>(glfwGetWindowUserPointer(pGlfwWindow));
    if (pApplication == nullptr) [[unlikely]] {
        return;
    }

    // Update viewport size according to the new framebuffer size.
    glViewport(0, 0, iWidth, iHeight);

    // Update projection matrix.
    pApplication->projectionMatrix = glm::perspective(
        glm::radians(verticalFov), static_cast<float>(iWidth) / iHeight, nearClipPlaneZ, farClipPlaneZ);
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

unsigned int Application::compileShader(const std::filesystem::path& pathToShader, bool bIsVertexShader) {
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
    glShaderSource(iShaderId, static_cast<int>(vCodesToAttach.size()), vCodesToAttach.data(), NULL);

    // Compile shader.
    glCompileShader(iShaderId);

    // See if there were any warnings/errors.
    int iSuccess = 0;
    std::array<char, 1024> infoLog = {0}; // NOLINT
    glGetShaderiv(iShaderId, GL_COMPILE_STATUS, &iSuccess);
    if (iSuccess == 0) {
        glGetShaderInfoLog(iShaderId, static_cast<int>(infoLog.size()), NULL, infoLog.data());
        throw std::runtime_error(std::format(
            "failed to compile shader from {}, error: {}", pathToShader.string(), infoLog.data()));
    }

    return iShaderId;
}
