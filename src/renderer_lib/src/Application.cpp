#include "Application.h"

// Standard.
#include <format>
#include <iostream>
#include <fstream>

// Custom.
#include "window/GLFW.hpp"
#include "ShaderIncluder.h"
#include "import/MeshImporter.h"
#include "window/ImGuiWindow.hpp"
#include "shader/ShaderUniformHelpers.hpp"

// External.
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

void Application::run() {
    // Create camera.
    pCamera = std::make_unique<Camera>();
    pCamera->setCameraMovementSpeed(10.0F); // NOLINT

    initWindow();
    setupImGui();
    initOpenGl();

    createFramebuffers();

    // Prepare environment map.
    iSkyboxCubemapId = TextureImporter::loadCubemap("res/skybox");
    iSkyboxShaderProgramId = compileSkyboxShaderProgram();
    pSkyboxMesh = std::move(MeshImporter::importMesh("res/skybox/skybox.glb")[0]);

    // Prepare post-processing shader program.
    iPostProcessingShaderProgramId = compilePostProcessShaderProgram();

    // Prepare screen quad.
    createScreenQuad();

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

void Application::createFramebuffers() {
    // Remove previous objects (if existed).
    glDeleteFramebuffers(1, &iRenderFramebufferId);
    glDeleteFramebuffers(1, &iPostProcessFramebufferId);
    glDeleteTextures(1, &iRenderFramebufferColorTextureId);
    glDeleteTextures(1, &iPostProcessFramebufferColorTextreId);
    glDeleteRenderbuffers(1, &iRenderFramebufferDepthStencilBufferId);

    // Create a framebuffer, a color texture and a depth/stencil buffer.
    glGenFramebuffers(1, &iRenderFramebufferId);
    glGenTextures(1, &iRenderFramebufferColorTextureId);
    glGenRenderbuffers(1, &iRenderFramebufferDepthStencilBufferId);

    // Bind them to the target to update them.
    glBindFramebuffer(GL_FRAMEBUFFER, iRenderFramebufferId);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, iRenderFramebufferColorTextureId);
    glBindRenderbuffer(GL_RENDERBUFFER, iRenderFramebufferDepthStencilBufferId);

    // Get window size.
    int iWidth = -1;
    int iHeight = -1;
    glfwGetWindowSize(pGLFWWindow, &iWidth, &iHeight);

    // Configure texture size/properties.
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, iMsaaSampleCount, GL_RGB8, iWidth, iHeight, GL_TRUE);

    // Attach color texture to framebuffer.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, iRenderFramebufferColorTextureId, 0);

    // Configure depth/stencil buffer size/properties.
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, iMsaaSampleCount, GL_DEPTH24_STENCIL8, iWidth, iHeight);

    // Attach depth/stencil buffer to framebuffer.
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, iRenderFramebufferDepthStencilBufferId);

    // Make sure framebuffer is complete.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Render framebuffer is not complete!");
    }

    // Unbind textures/buffers as we no longer need to update them.
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create framebuffer for post processing.
    glGenFramebuffers(1, &iPostProcessFramebufferId);

    // Create color texture for post processing framebuffer.
    glGenTextures(1, &iPostProcessFramebufferColorTextreId);

    // Bind framebuffer and texture to update them.
    glBindTexture(GL_TEXTURE_2D, iPostProcessFramebufferColorTextreId);
    glBindFramebuffer(GL_FRAMEBUFFER, iPostProcessFramebufferId);

    // Setup color texture.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Attach color texture to framebuffer.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, iPostProcessFramebufferColorTextreId, 0);

    // Make sure framebuffer is complete.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Post processing framebuffer is not complete!");
    }

    // Unbind textures/buffers as we no longer need to update them.
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::createScreenQuad() {
    std::vector<Vertex> vVertices;
    Vertex vertex;

    // Specify positions in the normalized device coordinates.

    vertex.position = glm::vec3(-1.0F, 1.0F, 0.0F);
    vertex.uv = glm::vec2(0.0F, 1.0F);
    vVertices.push_back(vertex);

    vertex.position = glm::vec3(-1.0F, -1.0F, 0.0F);
    vertex.uv = glm::vec2(0.0F, 0.0F);
    vVertices.push_back(vertex);

    vertex.position = glm::vec3(1.0F, -1.0F, 0.0F);
    vertex.uv = glm::vec2(1.0F, 0.0F);
    vVertices.push_back(vertex);

    vertex.position = glm::vec3(1.0F, 1.0F, 0.0F);
    vertex.uv = glm::vec2(1.0F, 1.0F);
    vVertices.push_back(vertex);

    pScreenQuadMesh = Mesh::create(std::move(vVertices), {0, 1, 2, 0, 2, 3});
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

        // Apply rotation from ImGui slider.
        setModelRotation(modelRotationToApply);

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
        if (pMesh->material.iDiffuseTextureId > 0) {
            macros.insert(ShaderProgramMacro::USE_DIFFUSE_TEXTURE);
        }
        if (pMesh->material.iMetallicRoughnessTextureId > 0) {
            macros.insert(ShaderProgramMacro::USE_METALLIC_ROUGHNESS_TEXTURE);
        }
        if (pMesh->material.iEmissionTextureId > 0) {
            macros.insert(ShaderProgramMacro::USE_EMISSION_TEXTURE);
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

    // Set light source position.
    vLightSources[0].setLightPosition(glm::vec3(cameraDistance * 2, cameraDistance * 2, cameraDistance * 2));
    vLightSources[1].setLightPosition(
        glm::vec3(-cameraDistance * 2, -cameraDistance * 2, -cameraDistance * 2));
}

void Application::setModelRotation(const glm::vec2& rotation) {
    for (const auto& [macros, shader] : meshesToDraw) {
        for (const auto& pMesh : shader.meshes) {
            pMesh->setWorldMatrix(
                MathHelpers::buildRotationMatrix(glm::vec3(rotation, 0.0F)) * glm::identity<glm::mat4x4>());
        }
    }
}

Application::ProfilingStatistics* Application::getProfilingStats() { return &stats; }

float* Application::getModelRotationToApply() { return glm::value_ptr(modelRotationToApply); }

float* Application::getFirstLightSourcePosition() { return vLightSources[0].getLightPosition(); }

float* Application::getSecondLightSourcePosition() { return vLightSources[1].getLightPosition(); }

float* Application::getEnvironmentIntensity() { return &environmentIntensity; }

float* Application::getAmbientLightIntensity() { return &ambientLightIntensity; }

void Application::drawNextFrame() {
    // Refresh culled object counter.
    stats.iCulledObjectsLastFrame = 0;

    // Set framebuffer to render the scene to.
    glBindFramebuffer(GL_FRAMEBUFFER, iRenderFramebufferId);

    // Clear color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get view and projection matrices.
    const auto viewMatrix = pCamera->getCameraProperties()->getViewMatrix();
    const auto projectionMatrix = pCamera->getCameraProperties()->getProjectionMatrix();

    // Draw meshes of each shader variation.
    for (const auto& [macros, shader] : meshesToDraw) {
        // Set shader program.
        glUseProgram(shader.iShaderProgramId);

        // Bind cubemap.
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iSkyboxCubemapId);

        // Set ambient light.
        ShaderUniformHelpers::setFloatToShader(
            shader.iShaderProgramId, "ambientLightIntensity", ambientLightIntensity);

        // Set environment intensity.
        ShaderUniformHelpers::setFloatToShader(
            shader.iShaderProgramId, "environmentIntensity", environmentIntensity);

        // Set light properties.
        for (size_t i = 0; i < vLightSources.size(); i++) {
            vLightSources[i].setToShader(shader.iShaderProgramId, i);
        }

        // Set camera position.
        ShaderUniformHelpers::setVector3ToShader(
            shader.iShaderProgramId,
            "cameraPositionInWorldSpace",
            pCamera->getCameraProperties()->getWorldLocation());

        // Set view/projection matrix.
        ShaderUniformHelpers::setMatrix4ToShader(
            shader.iShaderProgramId, "viewProjectionMatrix", projectionMatrix * viewMatrix);

        // Draw meshes.
        for (const auto& mesh : shader.meshes) {
            // Do frustum culling.
            if (!pCamera->getCameraProperties()->getCameraFrustum()->isAabbInFrustum(
                    mesh->aabb, *mesh->getWorldMatrix())) {
                stats.iCulledObjectsLastFrame += 1;
                continue;
            }

            // Set world/normal matrix.
            ShaderUniformHelpers::setMatrix4ToShader(
                shader.iShaderProgramId, "worldMatrix", *mesh->getWorldMatrix());
            ShaderUniformHelpers::setMatrix3ToShader(
                shader.iShaderProgramId, "normalMatrix", *mesh->getNormalMatrix());

            // Set vertex array object.
            glBindVertexArray(mesh->iVertexArrayObjectId);

            // Set element object.
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->iIndexBufferObjectId);

            // Set material properties.
            mesh->material.setToShader(shader.iShaderProgramId);

            // Submit a draw command.
            glDrawElements(GL_TRIANGLES, mesh->iIndexCount, GL_UNSIGNED_INT, nullptr);
        }
    }

    // Draw the skybox.
    drawSkybox();

    // Switch to post processing framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, iPostProcessFramebufferId);
    glClear(GL_COLOR_BUFFER_BIT);

    // Get created window size.
    int iWidth = -1;
    int iHeight = -1;
    glfwGetWindowSize(pGLFWWindow, &iWidth, &iHeight);

    // Resolve multisampled color texture to post processing framebuffer.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, iRenderFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, iPostProcessFramebufferId);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, iWidth, iHeight, 0, 0, iWidth, iHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Switch to default framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw a quad with the size of the screen.
    drawPostProcessingScreenQuad();

    // Finished submitting a new frame.
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
    const auto sPathToMacrosFile = std::filesystem::path("res/shaders/defined_macros.glsl");
    if (!std::filesystem::exists(sPathToMacrosFile.parent_path())) [[unlikely]] {
        throw std::runtime_error("expected the directory for shader resources to exist");
    }
    std::ofstream macrosFile(sPathToMacrosFile.string());
    if (!macrosFile.is_open()) [[unlikely]] {
        throw std::runtime_error("failed to create a file for predefined shader macros");
    }
    for (const auto& macro : macros) {
        macrosFile << "#define " + macroToText(macro) + "\n";
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

void Application::drawSkybox() {
    // Get view and projection matrices.
    const auto viewMatrix = pCamera->getCameraProperties()->getViewMatrix();
    const auto projectionMatrix = pCamera->getCameraProperties()->getProjectionMatrix();

    // Set shader program.
    glUseProgram(iSkyboxShaderProgramId);

    // Bind cubemap.
    glBindTexture(GL_TEXTURE_CUBE_MAP, iSkyboxCubemapId);

    // Set view/projection matrix.
    ShaderUniformHelpers::setMatrix4ToShader(
        iSkyboxShaderProgramId,
        "viewProjectionMatrix",
        projectionMatrix * glm::mat4(glm::mat3(
                               viewMatrix))); // remove translation to make skybox centered on camera location

    // Set vertex/index buffers.
    glBindVertexArray(pSkyboxMesh->iVertexArrayObjectId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pSkyboxMesh->iIndexBufferObjectId);

    // Disable backface culling to render cubemap (because the camera will be inside of that cube).
    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL); // change depth comparison to <=1.0F to render skybox because it has 1.0 depth
                            // (see shaders)

    {
        // Submit a draw command.
        glDrawElements(GL_TRIANGLES, pSkyboxMesh->iIndexCount, GL_UNSIGNED_INT, nullptr);
    }

    // Return backface culling and depth comparison function.
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS); // restore depth comparison function
}

void Application::drawPostProcessingScreenQuad() {
    // Set shader program.
    glUseProgram(iPostProcessingShaderProgramId);

    // Disable depth test because we need this quad to be rendered.
    glDisable(GL_DEPTH_TEST);

    {
        // Bind texture on which our scene was rendered.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, iPostProcessFramebufferColorTextreId);

        // Set vertex/index buffers.
        glBindVertexArray(pScreenQuadMesh->iVertexArrayObjectId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pScreenQuadMesh->iIndexBufferObjectId);

        // Submit a draw command.
        glDrawElements(GL_TRIANGLES, pScreenQuadMesh->iIndexCount, GL_UNSIGNED_INT, nullptr);
    }

    // Restore depth test.
    glEnable(GL_DEPTH_TEST);
}

unsigned int Application::compileSkyboxShaderProgram() {
    // Prepare shaders.
    const auto iVertexShaderId = compileShader("res/shaders/skybox_vertex.glsl", true);
    const auto iFragmentShaderId = compileShader("res/shaders/skybox_fragment.glsl", false);

    // Create shader program.
    const auto iShaderProgramId = glCreateProgram();

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

    return iShaderProgramId;
}

unsigned int Application::compilePostProcessShaderProgram() {
    // Prepare shaders.
    const auto iVertexShaderId = compileShader("res/shaders/post_process_vertex.glsl", true);
    const auto iFragmentShaderId = compileShader("res/shaders/post_process_fragment.glsl", false);

    // Create shader program.
    const auto iShaderProgramId = glCreateProgram();

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

    return iShaderProgramId;
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

    // Re-create framebuffer.
    pApplication->createFramebuffers();
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

#undef max

    // Scale UI (source: https://gist.github.com/benpm/21afb58f2c8dfdbf881ca90c76ad602e)
    float monScaleX = 0.0F;
    float monScaleY = 0.0F;
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &monScaleX, &monScaleY);
    int dpiScale = std::max((int)monScaleX, (int)monScaleY);
    ImGui::GetStyle().ScaleAllSizes((float)dpiScale);
    ImGui::GetIO().FontGlobalScale = (float)dpiScale;
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    fontConfig.SizePixels = 16.0f * dpiScale; // NOLINT
    ImGui::GetIO().Fonts->AddFontDefault(&fontConfig);
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
