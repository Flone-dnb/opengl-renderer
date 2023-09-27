#pragma once

// Standard.
#include <stdexcept>
#include <string>

// External.
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
    }
};
