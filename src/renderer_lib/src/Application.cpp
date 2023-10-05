#include "Application.h"

// Standard.
#include <format>
#include <iostream>
#include <fstream>

// Custom.
#include "GLFW.hpp"
#include "ShaderIncluder.h"
#include "MeshImporter.h"
#include "ImGuiWindow.hpp"

// External.
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
    setupImGui();
    initOpenGl();

    mainLoop();

    shutdownImGui();
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

    // Disable VSync.
    glfwSwapInterval(0);

    // Enable MSAA.
    glEnable(GL_MULTISAMPLE);
}

void Application::mainLoop() {
    // Used for tick.
    float currentTimeInSec = 0.0F;
    float prevTimeInSec = static_cast<float>(glfwGetTime());

    while (glfwWindowShouldClose(pGLFWWindow) == 0) {
        // Process window events.
        glfwPollEvents();

        // Start drawing the Dear ImGui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiWindow::drawWindow(this);

        // Notify camera.
        currentTimeInSec = static_cast<float>(glfwGetTime());
        pCamera->onBeforeNewFrame(currentTimeInSec - prevTimeInSec);
        prevTimeInSec = currentTimeInSec;

        drawNextFrame();

        // Finish drawing the Dear ImGui frame.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap back/front buffers.
        glfwSwapBuffers(pGLFWWindow);
    }
}

void Application::prepareScene(const std::filesystem::path& pathToModel) {
    // Clear current scene.
    meshesToDraw.clear();

    // Import meshes from file.
    auto vImportedMeshes = MeshImporter::importMesh(pathToModel);

    // See which macros we need to define.
    std::unordered_set<ShaderProgramMacro> macros;
    for (const auto& pMesh : vImportedMeshes) {
        if (pMesh->iDiffuseTextureId > 0) {
            macros.insert(ShaderProgramMacro::USE_DIFFUSE_TEXTURE);
        }
    }

    // Prepare shader program for the specified macros.
    prepareShaderProgram(macros);

    float cameraDistance = 0.0F;

    // Add to meshes to be drawn.
    for (auto& pMesh : vImportedMeshes) {
        // Calculate camera's distance to capture the meshes.
        const auto xBound = std::abs(pMesh->aabb.extents.x) * 2;
        const auto yBound = std::abs(pMesh->aabb.extents.y) * 2;
        const auto zBound = std::abs(pMesh->aabb.extents.z) * 2;
        if (xBound > cameraDistance) {
            cameraDistance = xBound;
        }
        if (yBound > cameraDistance) {
            cameraDistance = yBound;
        }
        if (zBound > cameraDistance) {
            cameraDistance = zBound;
        }

        // Add mesh to be drawn.
        meshesToDraw[macros].meshes.insert(std::move(pMesh));
    }

    // Set camera's position/rotation.
    pCamera->setLocation(glm::vec3(0.0F, 0.0F, cameraDistance * 2));
    pCamera->setFreeCameraRotation(glm::vec3(0.0F, 0.0F, -1.0F));
}

Application::ProfilingStatistics* Application::getProfilingStats() { return &stats; }

void Application::drawNextFrame() {
    // Refresh culled object counter.
    stats.iCulledObjectsLastFrame = 0;

    // Clear color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw meshes of each shader variation.
    for (const auto& [macros, shader] : meshesToDraw) {
        glUseProgram(shader.iShaderProgramId);

        // Get view and projection matrices.
        const auto viewMatrix = pCamera->getCameraProperties()->getViewMatrix();
        const auto projectionMatrix = pCamera->getCameraProperties()->getProjectionMatrix();

        // Set view/projection matrix.
        const auto iViewProjectionMatrixLocation =
            glGetUniformLocation(shader.iShaderProgramId, "viewProjectionMatrix");
        if (iViewProjectionMatrixLocation < 0) [[unlikely]] {
            throw std::runtime_error("unable to get location for view/projection matrix");
        }
        const auto viewProjectionMatrix = projectionMatrix * viewMatrix;
        glUniformMatrix4fv(iViewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

        for (const auto& mesh : shader.meshes) {
            // Do frustum culling.
            if (!pCamera->getCameraProperties()->getCameraFrustum()->isAabbInFrustum(
                    mesh->aabb, mesh->worldMatrix)) {
                stats.iCulledObjectsLastFrame += 1;
                continue;
            }

            // Set world matrix.
            const auto iWorldMatrixLocation = glGetUniformLocation(shader.iShaderProgramId, "worldMatrix");
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

            // Set texture wrapping.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // Set texture filtering.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_MAG_FILTER,
                GL_LINEAR); // no need to set `MIPMAP` option since magnification does not use mipmaps

            // Enable anisotropic texture filtering (core in OpenGL 4.6 which we are using).
            float maxSupportedAnisotropy = 0.0F;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxSupportedAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxSupportedAnisotropy);

            // Submit a draw command.
            glDrawElements(GL_TRIANGLES, mesh->iIndexCount, GL_UNSIGNED_INT, nullptr);
        }
    }

    onFrameSubmitted();
}

void Application::prepareShaderProgram(const std::unordered_set<ShaderProgramMacro>& macros) {
    // See if a shader program with these macros was already compiled.
    if (meshesToDraw.find(macros) != meshesToDraw.end()) {
        // Nothing to do.
        return;
    }

    // because I haven't found a way to properly define macros for GLSL shaders from C/C++ code
    // using OpenGL functionality I decided to make shaders to include a special non-existing file
    // that will contain all defined macros, multi-threaded shader compilation with this approach
    // is impossible but it's OK because this project is just for learning purposes,
    // now I will create the file with macrosand delete it after compilation
    static std::mutex predefinedMacrosMutex;
    std::scoped_lock guard(predefinedMacrosMutex); // using a static mutex to create a critical section here
                                                   // because we create and delete a temporary file with
                                                   // predefined shader macros, although at the moment
                                                   // of writing this we don't use multi-threaded shader
                                                   // compilation I'm adding this static mutex to not
                                                   // shoot myself in the foot if in the future I will add
                                                   // multi-threaded shader compilation

    // Create a temporary file with defined macros.
    const auto sPathToMacrosFile = std::string("res/shaders/defined_macros.glsl");
    std::ofstream macrosFile(sPathToMacrosFile);
    if (!macrosFile.is_open()) [[unlikely]] {
        throw std::runtime_error("failed to create a file for predefined shader macros");
    }
    for (const auto& macro : macros) {
        macrosFile << "#define " + macroToText(macro);
    }
    macrosFile.close();

    // Prepare shaders.
    const auto iVertexShaderId = compileShader("res/shaders/vertex.glsl", true);
    const auto iFragmentShaderId = compileShader("res/shaders/fragment.glsl", false);

    // Remove the temporary file with macros.
    std::filesystem::remove(sPathToMacrosFile);

    // Create shader program.
    auto& shaderProgram = meshesToDraw[macros];
    shaderProgram.iShaderProgramId = glCreateProgram();

    // Attach shaders to shader program.
    glAttachShader(shaderProgram.iShaderProgramId, iVertexShaderId);
    glAttachShader(shaderProgram.iShaderProgramId, iFragmentShaderId);

    // Link shaders together.
    glLinkProgram(shaderProgram.iShaderProgramId);

    // See if there were any linking errors.
    int iSuccess = 0;
    std::array<char, 1024> infoLog = {0}; // NOLINT
    glGetProgramiv(shaderProgram.iShaderProgramId, GL_LINK_STATUS, &iSuccess);
    if (iSuccess == 0) {
        glGetProgramInfoLog(
            shaderProgram.iShaderProgramId, static_cast<int>(infoLog.size()), NULL, infoLog.data());
        throw std::runtime_error(std::format("failed to link shader program, error: {}", infoLog.data()));
    }

    // Delete shaders since we don't need them anymore.
    glDeleteShader(iVertexShaderId);
    glDeleteShader(iFragmentShaderId);
}

void Application::onFrameSubmitted() {
    using namespace std::chrono;

    // Increment frame count.
    static size_t iTotalFramesSubmittedLastSecond = 0;
    iTotalFramesSubmittedLastSecond += 1;

    // Calculate how much time has passed since we updated our FPS counter last time.
    const auto iTimeSinceFpsUpdateInSec =
        duration_cast<seconds>(steady_clock::now() - stats.timeAtLastFpsUpdate).count();

    if (iTimeSinceFpsUpdateInSec >= 1) {
        // Update FPS counter.
        stats.iFramesPerSecond = iTotalFramesSubmittedLastSecond;
        iTotalFramesSubmittedLastSecond = 0;

        stats.timeAtLastFpsUpdate = steady_clock::now();
    }
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

void Application::setupImGui() {
    // Make sure window was initialized.
    if (pGLFWWindow == nullptr) [[unlikely]] {
        throw std::runtime_error("expected the window to be initialized");
    }

    // Setup Dear ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& imguiIo = ImGui::GetIO();
    imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Platform/Renderer backends.
    ImGui_ImplGlfw_InitForOpenGL(pGLFWWindow, true);
    ImGui_ImplOpenGL3_Init();
}

void Application::shutdownImGui() {
    // Shutdown ImGui.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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
