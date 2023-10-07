#pragma once

// Standard.
#include <array>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

// Custom.
#include "math/GLMath.hpp"
#include "camera/Camera.h"
#include "Mesh.h"
#include "ShaderProgramMacro.hpp"
#include "LightSource.h"

struct GLFWwindow;

/** Groups meshes that use the same shader program. */
struct ShaderMeshGroup {
    /** ID of the shader program. */
    unsigned int iShaderProgramId = 0;

    /** Meshes that use shader program @ref iShaderProgramId. */
    std::unordered_set<std::unique_ptr<Mesh>> meshes;
};

/** Basic OpenGL application. */
class Application {
public:
    /** Groups various statistics such as FPS. */
    struct ProfilingStatistics {
        /** The total number of frames drawn last second. */
        size_t iFramesPerSecond = 0;

        /** The total number of objects that was culled and not submitted for drawing. */
        size_t iCulledObjectsLastFrame = 0;

        /** Last time when @ref iFramesPerSecond was updated. */
        std::chrono::steady_clock::time_point timeAtLastFpsUpdate = std::chrono::steady_clock::now();
    };

    /**
     * Sets the specified matrix to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param matrix           Matrix to set.
     */
    static void setMatrix4ToShader(
        unsigned int iShaderProgramId, const std::string& sUniformName, const glm::mat4x4& matrix);

    /**
     * Sets the specified matrix to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param matrix           Matrix to set.
     */
    static void setMatrix3ToShader(
        unsigned int iShaderProgramId, const std::string& sUniformName, const glm::mat3x3& matrix);

    /**
     * Sets the specified vector to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param vector           Vector to set.
     */
    static void setVector3ToShader(
        unsigned int iShaderProgramId, const std::string& sUniformName, const glm::vec3& vector);

    /**
     * Sets the specified float value to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param value            Value to set.
     */
    static void setFloatToShader(unsigned int iShaderProgramId, const std::string& sUniformName, float value);

    /** Runs the application. */
    void run();

    /**
     * Prepares a scene with meshes to draw (fills @ref meshesToDraw).
     *
     * @remark Clears old displayed models (if existed).
     *
     * @param pathToModel Path to the file to import and display.
     */
    void prepareScene(const std::filesystem::path& pathToModel);

    /**
     * Returns app statistics.
     *
     * @remark Do not delete (free) returned pointer.
     *
     * @return App statistics.
     */
    ProfilingStatistics* getProfilingStats();

    /**
     * Value for ImGui slider to modify rotation.
     *
     * @return Pointer that points to the start of the model rotation vector that will be applied.
     */
    float* getModelRotationToApply();

    /**
     * Returns light source position to be modified in ImGui slider.
     *
     * @return Pointer that points to the start of the light position vector.
     */
    float* getLightSourcePosition();

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
     * GLFW callback.
     *
     * @param pGlfwWindow The window that received the event.
     * @param iButton     The mouse button that was pressed or released.
     * @param iAction     One of GLFW_PRESS or GLFW_RELEASE.
     * @param iMods 	     Bit field describing which modifier keys were held down.
     */
    static void glfwWindowMouseCallback(GLFWwindow* pGlfwWindow, int iButton, int iAction, int iMods);

    /**
     * GLFW callback.
     *
     * @param pGlfwWindow The window that received the event.
     * @param xPos        The new x-coordinate, in screen coordinates, of the cursor.
     * @param yPos        The new y-coordinate, in screen coordinates, of the cursor.
     */
    static void glfwWindowMouseCursorPosCallback(GLFWwindow* pGlfwWindow, double xPos, double yPos);

    /**
     * Creates a new shader, assigns it to the OpenGL context, compiles it and returns shader's ID.
     *
     * @param pathToShader    Path to shader code on disk.
     * @param bIsVertexShader `true` if this is a vertex shader, `false` if fragment shader.
     *
     * @return Created shader's ID.
     */
    static unsigned int compileShader(const std::filesystem::path& pathToShader, bool bIsVertexShader);

    /**
     * Sets rotation of all displayed (imported) meshes.
     *
     * @param rotation Rotation.
     */
    void setModelRotation(const glm::vec2& rotation);

    /**
     * Setups the Dear ImGui library.
     *
     * @warning Expects that @ref pGLFWWindow is initialized.
     */
    void setupImGui();

    /** Deinitializes the Dear ImGui library after @ref setupImGui. */
    static void shutdownImGui();

    /**
     * Changes cursor's visibility.
     *
     * @param bIsVisible Whether the cursor should be visible or not.
     */
    void setCursorVisibility(bool bIsVisible) const;

    /** Initializes GLFW. */
    void initWindow();

    /** Initializes rendering. */
    static void initOpenGl();

    /** Processes window messages and does the rendering. */
    void mainLoop();

    /** Draws next frame. */
    void drawNextFrame();

    /**
     * Checks that a shader program with the specified properties in @ref meshesToDraw exists
     * and if not creates and compiles one.
     *
     * @param macros Macros that should be defined for a shader program.
     */
    void prepareShaderProgram(const std::unordered_set<ShaderProgramMacro>& macros);

    /** Updates @ref stats. */
    void onFrameSubmitted();

    /**
     * Rotation for all displayed (imported) models to apply.
     *
     * @remark Modified from ImGui slider.
     */
    glm::vec2 modelRotationToApply = glm::vec2(0.0F, 0.0F);

    /** Virtual camera. */
    std::unique_ptr<Camera> pCamera;

    /** Stores pairs of "macros of a shader program" - "meshes that use this shader program". */
    std::unordered_map<
        std::unordered_set<ShaderProgramMacro>,
        ShaderMeshGroup,
        ShaderProgramMacroUnorderedSetHash>
        meshesToDraw;

    /** Scene's light source. */
    LightSource lightSource;

    /** Color of the ambient lighting. */
    glm::vec3 ambientColor = glm::vec3(0.1F, 0.1F, 0.1F); // NOLINT

    /** Various statistics for profiling. */
    ProfilingStatistics stats;

    /** GLFW window. */
    GLFWwindow* pGLFWWindow = nullptr;

    /** Used to calculate mouse movement offset. */
    double lastMousePosX = 0.0;

    /** Used to calculate mouse movement offset. */
    double lastMousePosY = 0.0;

    /** Camera rotation multiplier. */
    const double cameraRotationSensitivity = 0.1;

    /** `true` if mouse cursor is hidden, `false `otherwise. */
    bool bIsMouseCursorCaptured = false;
};
