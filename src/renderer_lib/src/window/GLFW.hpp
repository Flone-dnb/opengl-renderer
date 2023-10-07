#pragma once

// Standard.
#include <stdexcept>
#include <string>

// External.
#include "glad/glad.h" // include GLAD before GLFW
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#undef MessageBox
#undef IGNORE

inline void glfwErrorCallback(int iErrorCode, const char* pDescription) {
    throw std::runtime_error("GLFW error (" + std::to_string(iErrorCode) + "): " + std::string(pDescription));
}

/** Singleton helper class to globally initialize/terminate GLFW. */
class GLFW {
public:
    GLFW(const GLFW&) = delete;
    GLFW& operator=(const GLFW&) = delete;

    /** Terminates GLFW. */
    ~GLFW() { glfwTerminate(); }

    /**
     * Creates a static GLFW instance and return instance if not created before.
     *
     * @return Singleton.
     */
    static GLFW& get() {
        static GLFW glfw;
        return glfw;
    }

private:
    /** Initializes GLFW. */
    GLFW() {
        glfwSetErrorCallback(glfwErrorCallback);

        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error("failed to initialize GLFW");
        }

        // Set OpenGL hints.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // NOLINT
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); // NOLINT
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 8); // NOLINT
#if defined(DEBUG)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    }
};
