#include "Application.h"

// Standard.
#include <format>
#include <iostream>
#include <fstream>

// Custom.
#include "GLFW.hpp"
#include "ShaderIncluder.h"

void Application::run() {
    initWindow();
    initOpenGl();
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
    // Specify clear color.
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    // Set vertex buffer to context.
    prepareVertexBuffer();

    // Set shaders to context.
    prepareShaders();
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

void Application::drawNextFrame() const {
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT);

    // Specify which shaders we will use.
    glUseProgram(iShaderProgramId);

    // Specify vertex array object.
    glBindVertexArray(iVertexArrayObjectId);

    // Draw using the specified vertex array object.
    glDrawArrays(GL_TRIANGLES, 0, vVertices.size());
}

void Application::prepareVertexBuffer() {
    // Create vertex buffer object (VBO).
    glGenBuffers(1, &iVertexBufferObjectId);

    // Create vertex array object (VAO).
    glGenVertexArrays(1, &iVertexArrayObjectId);

    // Bind our vertex array object to the OpenGL context.
    glBindVertexArray(iVertexArrayObjectId);

    // Set our buffer to "array" buffer target in OpenGL context
    // (i.e. attach a VBO to our VAO).
    glBindBuffer(GL_ARRAY_BUFFER, iVertexBufferObjectId);

    // Copy vertices to the buffer.
    glBufferData(
        GL_ARRAY_BUFFER,
        vVertices.size() * sizeof(vVertices[0]),
        vVertices.data(),
        GL_STATIC_DRAW); // `STATIC` because the data will not be changed
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

    // Specify vertex attributes.
    setVertexAttributes();
}

void Application::setVertexAttributes() {
    // Specify position.
    glVertexAttribPointer(
        0,                 // attribute index (layout location)
        3,                 // number of components
        GL_FLOAT,          // type of component
        GL_FALSE,          // whether data should be normalized or not
        sizeof(glm::vec3), // stride (size in bytes between elements)
        0);                // beginning offset
    glEnableVertexAttribArray(0);
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
