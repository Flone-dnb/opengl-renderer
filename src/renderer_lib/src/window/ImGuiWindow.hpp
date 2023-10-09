#pragma once

// Standard.
#include <filesystem>

// Custom.
#include "Application.h"
#include "import/TextureImporter.h"

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

            ImGui::Checkbox("flip textures vertically", &TextureImporter::bFlipTexturesVertically);

            if (ImGui::Button("select GLTF/GLB file to display")) {
                const auto vPickedPaths = pfd::open_file(
                                              "Select GLTF/GLB file to display",
                                              std::filesystem::current_path().string(),
                                              {"GLTF", "*.gltf *.glb"},
                                              pfd::opt::none)
                                              .result();
                if (!vPickedPaths.empty()) {
                    pApp->prepareScene(vPickedPaths[0]);
                }
            }

            ImGui::SeparatorText("Controls");

            ImGui::Text("hold right mouse button and WASDEQ to move/rotate");

            ImGui::SeparatorText("Model");

            ImGui::PushItemWidth(ImGui::GetFontSize() * 15.0F);                                      // NOLINT
            ImGui::SliderFloat2("model pitch / yaw", pApp->getModelRotationToApply(), 0.0F, 360.0F); // NOLINT

            ImGui::SeparatorText("Lighting");

            ImGui::SliderFloat3(
                "light #1 position", pApp->getFirstLightSourcePosition(), -30.0F, 30.0F); // NOLINT
            ImGui::SliderFloat3(
                "light #2 position", pApp->getSecondLightSourcePosition(), -30.0F, 30.0F); // NOLINT
            ImGui::SliderFloat(
                "environment intensity", pApp->getEnvironmentIntensity(), 0.0F, 1.0F); // NOLINT

            ImGui::SeparatorText("Statistics");

            ImGui::Text("FPS: %zu", pApp->getProfilingStats()->iFramesPerSecond);
            ImGui::Text("Culled objects: %zu", pApp->getProfilingStats()->iCulledObjectsLastFrame);
        }

        ImGui::End();
    }
};
