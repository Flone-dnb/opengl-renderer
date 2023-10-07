#pragma once

// Standard.
#include <filesystem>

// Custom.
#include "Application.h"

// External.
#include "imgui.h"
#include "portable-file-dialogs.h"

/** Displays a small ImGui window. */
class ImGuiWindow {
public:
    ImGuiWindow() = delete;

    /**
     * Queues ImGui widgets to be drawn.
     *
     * @param pApp Application that uses this window.
     */
    static inline void drawWindow(Application* pApp) {
        // Prepare to create a window.
        const auto pMainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
            ImVec2(pMainViewport->WorkPos.x, pMainViewport->WorkPos.y), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_FirstUseEver); // NOLINT

        // Start window.
        if (!ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar)) {
            ImGui::End();
            return;
        }

        {
            ImGui::SeparatorText("Import");

            ImGui::Checkbox("flip textures vertically", &Application::bFlipTexturesVertically);

            if (ImGui::Button("select GLTF/GLB file to display")) {
                const auto vPickedPaths = pfd::open_file(
                                              "Select GLTF/GLB file to display",
                                              std::filesystem::current_path(),
                                              {"GLTF (*.gltf *.glb)"},
                                              pfd::opt::none)
                                              .result();
                // Convert result.
                std::vector<std::filesystem::path> vSelectedPaths(vPickedPaths.size());
                for (size_t i = 0; i < vPickedPaths.size(); i++) {
                    vSelectedPaths[i] = vPickedPaths[i];
                }

                if (!vSelectedPaths.empty()) {
                    pApp->prepareScene(vSelectedPaths[0]);
                }
            }

            ImGui::SeparatorText("Controls");

            ImGui::Text("hold right mouse button and WASDEQ to move/rotate");

            ImGui::SeparatorText("Model");

            ImGui::PushItemWidth(ImGui::GetFontSize() * 15.0F);                                      // NOLINT
            ImGui::SliderFloat2("model pitch / yaw", pApp->getModelRotationToApply(), 0.0F, 360.0F); // NOLINT

            ImGui::SeparatorText("Lighting");

            ImGui::SliderFloat3("light position", pApp->getLightSourcePosition(), -30.0F, 30.0F); // NOLINT

            ImGui::SeparatorText("Statistics");

            ImGui::Text("FPS: %zu", pApp->getProfilingStats()->iFramesPerSecond);
            ImGui::Text("Culled objects: %zu", pApp->getProfilingStats()->iCulledObjectsLastFrame);
        }

        ImGui::End();
    }
};
