#pragma once

// Standard.
#include <string>

// Custom.
#include "GLMath.hpp"

class GLFWwindow;

/** Basic OpenGL application. */
class Application {
public:
    /** Runs the application. */
    void run();

private:
    /** Initializes GLFW. */
    void initWindow();

    /** Processes window messages and does the rendering. */
    void mainLoop();

    /** GLFW window. */
    GLFWwindow* pGLFWWindow = nullptr;
};
