#pragma once

// External.
#include "imgui.h"

/** Displays a small ImGui window. */
class ImGuiWindow {
public:
    ImGuiWindow() = delete;

    /**
     * Queues ImGui widgets to be drawn.
     *
     * @param iFramesPerSecond Current FPS.
     */
    static inline void drawWindow(size_t iFramesPerSecond) {
        // Prepare to create a window.
        const auto pMainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
            ImVec2(pMainViewport->WorkPos.x, pMainViewport->WorkPos.y), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver); // NOLINT

        // Start window.
        if (!ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar)) {
            ImGui::End();
            return;
        }

        {
            ImGui::Text("FPS: %zu", iFramesPerSecond);
            // TODO
        }

        ImGui::End();
    }
};
