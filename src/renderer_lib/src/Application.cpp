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
    // Create camera.
    pCamera = std::make_unique<Camera>();
    pCamera->setCameraMovementSpeed(10.0F); // NOLINT

    initWindow();
    initOpenGl();
    prepareScene();
    mainLoop();
}

void Application::initWindow() {
    // Initialize GLFW.
    GLFW::get();

    // Create maximized window.
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Get main monitor.
    const auto pMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

    // Create GLFW window.
    pGLFWWindow =
        glfwCreateWindow(pMode->width, pMode->height, "OpenGL", nullptr, nullptr); // NOLINT: magic number
    if (pGLFWWindow == nullptr) {
        throw std::runtime_error("failed to create window");
    }

    // Get created window size.
    int iWidth = -1;
    int iHeight = -1;
    glfwGetWindowSize(pGLFWWindow, &iWidth, &iHeight);

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
    glViewport(0, 0, iWidth, iHeight);

    // Update camera's aspect ratio.
    pCamera->getCameraProperties()->setAspectRatio(iWidth, iHeight);

    // Bind to framebuffer size change event.
    glfwSetFramebufferSizeCallback(pGLFWWindow, Application::glfwFramebufferResizeCallback);

    // Bind to mouse input.
    glfwSetMouseButtonCallback(pGLFWWindow, Application::glfwWindowMouseCallback);

    // Bind to mouse move.
    glfwSetCursorPosCallback(pGLFWWindow, Application::glfwWindowMouseCursorPosCallback);

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

    // Enable back face culling.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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
    // Used for tick.
    float currentTimeInSec = 0.0F;
    float prevTimeInSec = static_cast<float>(glfwGetTime());

    while (glfwWindowShouldClose(pGLFWWindow) == 0) {
        // Process window events.
        glfwPollEvents();

        // Notify camera.
        currentTimeInSec = static_cast<float>(glfwGetTime());
        pCamera->onBeforeNewFrame(currentTimeInSec - prevTimeInSec);
        prevTimeInSec = currentTimeInSec;

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
}

void Application::drawNextFrame() const {
    // Clear color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set shaders.
    glUseProgram(iShaderProgramId);

    // Get view and projection matrices.
    const auto viewMatrix = pCamera->getCameraProperties()->getViewMatrix();
    const auto projectionMatrix = pCamera->getCameraProperties()->getProjectionMatrix();

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
    // Get app pointer.
    const auto pApplication = static_cast<Application*>(glfwGetWindowUserPointer(pGlfwWindow));
    if (pApplication == nullptr) [[unlikely]] {
        return;
    }

    // Update viewport size according to the new framebuffer size.
    glViewport(0, 0, iWidth, iHeight);

    // Update camera's aspect ratio.
    pApplication->pCamera->getCameraProperties()->setAspectRatio(iWidth, iHeight);
}

void Application::glfwWindowKeyboardCallback(
    GLFWwindow* pGlfwWindow, int iKey, int iScancode, int iAction, int iMods) {
    // Ignore "repeat" actions.
    if (iAction == GLFW_REPEAT) {
        return;
    }

    // Get app pointer.
    const auto pApplication = static_cast<Application*>(glfwGetWindowUserPointer(pGlfwWindow));
    if (pApplication == nullptr) [[unlikely]] {
        return;
    }

    // See if we need to close the window.
    if (iKey == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(pGlfwWindow, 1);
    }

    // Next, process movement.
    if (!pApplication->bIsMouseCursorCaptured) {
        // Reset all input (if there was any input).
        pApplication->pCamera->setFreeCameraForwardMovement(0.0F);
        pApplication->pCamera->setFreeCameraRightMovement(0.0F);
        return;
    }

    if (iKey == GLFW_KEY_W) {
        pApplication->pCamera->setFreeCameraForwardMovement(iAction == GLFW_PRESS ? 1.0F : 0.0F);
    } else if (iKey == GLFW_KEY_S) {
        pApplication->pCamera->setFreeCameraForwardMovement(iAction == GLFW_PRESS ? -1.0F : 0.0F);
    } else if (iKey == GLFW_KEY_D) {
        pApplication->pCamera->setFreeCameraRightMovement(iAction == GLFW_PRESS ? 1.0F : 0.0F);
    } else if (iKey == GLFW_KEY_A) {
        pApplication->pCamera->setFreeCameraRightMovement(iAction == GLFW_PRESS ? -1.0F : 0.0F);
    } else if (iKey == GLFW_KEY_E) {
        pApplication->pCamera->setFreeCameraWorldUpMovement(iAction == GLFW_PRESS ? 1.0F : 0.0F);
    } else if (iKey == GLFW_KEY_Q) {
        pApplication->pCamera->setFreeCameraWorldUpMovement(iAction == GLFW_PRESS ? -1.0F : 0.0F);
    }
}

void Application::glfwWindowMouseCallback(GLFWwindow* pGlfwWindow, int iButton, int iAction, int iMods) {
    // Ignore "repeat" actions.
    if (iAction == GLFW_REPEAT) {
        return;
    }

    // Get app pointer.
    const auto pApplication = static_cast<Application*>(glfwGetWindowUserPointer(pGlfwWindow));
    if (pApplication == nullptr) [[unlikely]] {
        return;
    }

    if (iButton == GLFW_MOUSE_BUTTON_RIGHT) {
        const auto bIsPressed = iAction == GLFW_PRESS;
        pApplication->setCursorVisibility(!bIsPressed);
        pApplication->bIsMouseCursorCaptured = bIsPressed;
    }
}

void Application::glfwWindowMouseCursorPosCallback(GLFWwindow* pGlfwWindow, double xPos, double yPos) {
    // Get app pointer.
    const auto pApplication = static_cast<Application*>(glfwGetWindowUserPointer(pGlfwWindow));
    if (pApplication == nullptr) [[unlikely]] {
        return;
    }

    // Calculate delta.
    const auto deltaX = xPos - pApplication->lastMousePosX;
    const auto deltaY = pApplication->lastMousePosY - yPos;

    // Save current position to calculate delta on next movement.
    pApplication->lastMousePosX = xPos;
    pApplication->lastMousePosY = yPos;

    if (!pApplication->bIsMouseCursorCaptured) {
        return;
    }

    // Apply movement to camera rotation.
    auto currentRotation = pApplication->pCamera->getFreeCameraRotation();
    currentRotation.y -= static_cast<float>(deltaX * pApplication->cameraRotationSensitivity); // yaw
    currentRotation.x += static_cast<float>(deltaY * pApplication->cameraRotationSensitivity); // pitch
    pApplication->pCamera->setFreeCameraRotation(currentRotation);
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

void Application::setCursorVisibility(bool bIsVisible) const {
    if (bIsVisible) {
        if (glfwRawMouseMotionSupported() == GLFW_TRUE) {
            glfwSetInputMode(pGLFWWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        }
        glfwSetInputMode(pGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(pGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported() == GLFW_TRUE) {
            glfwSetInputMode(pGLFWWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        } else {
            throw std::runtime_error("raw mouse motion is not supported");
        }
    }
}
