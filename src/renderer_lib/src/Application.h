#pragma once

// Standard.
#include <array>
#include <string>
#include <vector>
#include <filesystem>

// Custom.
#include "GLMath.hpp"
#include "Mesh.h"

class GLFWwindow;

/** Basic OpenGL application. */
class Application {
public:
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

    /** Runs the application. */
    void run();

private:
    /**
     * GLFW callback that's called after the framebuffer size was changed.
     *
     * @param pGlfwWindow Window.
     * @param iWidth      New width.
     * @param iHeight     New height.
     */
    static void glfwFramebufferResizeCallback(GLFWwindow* pGlfwWindow, int iWidth, int iHeight);

    /**
     * GLFW callback that's called when a keyboard input event was received and is now being processed.
     *
     * @param pGlfwWindow The window that received the event.
     * @param iKey        The keyboard key that was pressed or released.
     * @param iScancode   The system-specific scancode of the key.
     * @param iAction     GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param iMods       Bit field describing which modifier keys were held down.
     */
    static void
    glfwWindowKeyboardCallback(GLFWwindow* pGlfwWindow, int iKey, int iScancode, int iAction, int iMods);

    /**
     * Creates a new shader, assigns it to the OpenGL context, compiles it and returns shader's ID.
     *
     * @param pathToShader    Path to shader code on disk.
     * @param bIsVertexShader `true` if this is a vertex shader, `false` if fragment shader.
     *
     * @return Created shader's ID.
     */
    static unsigned int compileShader(const std::filesystem::path& pathToShader, bool bIsVertexShader);

    /** Initializes GLFW. */
    void initWindow();

    /** Initializes rendering. */
    void initOpenGl();

    /** Processes window messages and does the rendering. */
    void mainLoop();

    /** Prepares a scene with meshes to draw (fills @ref vMeshesToDraw).  */
    void prepareScene();

    /** Draws next frame. */
    void drawNextFrame() const;

    /**
     * Creates and compiles shaders into a shader program, assigns it to the OpenGL context and stores its ID
     * in @ref iShaderProgramId.
     */
    void prepareShaders();

    /** Stores meshes to draw. */
    std::vector<std::unique_ptr<Mesh>> vMeshesToDraw;

    /** ID of the shader program that contains all other shaders attached to it. */
    unsigned int iShaderProgramId = 0;

    /** GLFW window. */
    GLFWwindow* pGLFWWindow = nullptr;
};
