#include "Application.h"

// Standard.
#include <format>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <fstream>

// Custom.
#include "GLFW.hpp"

void Application::run() {
    initWindow();
    mainLoop();
}

void Application::initWindow() {
    // Initialize GLFW.
    GLFW::get();

    // Create GLFW window.
    pGLFWWindow = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr); // NOLINT: magic number
    if (pGLFWWindow == nullptr) {
        throw std::runtime_error("failed to create window");
    }
}

void Application::mainLoop() {
    while (glfwWindowShouldClose(pGLFWWindow) == 0) {
        glfwPollEvents();
    }
}
