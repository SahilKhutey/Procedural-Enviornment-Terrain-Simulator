#pragma once

#include "../debug/EngineValidator.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>

#include "../terrain/TerrainGenerator.h"
#include "../terrain/TerrainManager.h"
#include "../environment/EnvironmentManager.h"
#include "../presentation/PresentationManager.h"
#include "../camera/Camera.h"

class GUIManager {
public:
    TestRunner testRunner;
    bool showTests = false;
    float frameTimes[100] = {};
    int frameOffset = 0;

    void Render(TerrainGenerator& generator, TerrainManager& terrainManager, 
                EnvironmentManager& envManager, PresentationManager& presManager,
                Camera& camera,
                float& exposure, float& bloomStrength, glm::vec3& tint, bool& wireframe, float deltaTime) {
        ImGui::Begin("Simulation Control");
        
        if (ImGui::BeginTabBar("EngineTabs")) {
            
            // --- TAB: PERFORMANCE ---
            // ...

            // --- TAB: DIAGNOSTICS ---
            if (ImGui::BeginTabItem("Diagnostics")) {
                if (ImGui::Button("Run System Audit")) {
                    EngineValidator::RunAllTests(testRunner, generator, terrainManager, camera);
                    showTests = true;
                }

                if (showTests) {
                    ImGui::Text("Result: %.0f%% Passed", testRunner.GetPassPercentage() * 100.0f);
                    ImGui::Separator();
                    for (auto& r : testRunner.results) {
                        ImGui::TextColored(r.passed ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), 
                                         r.passed ? "[PASS]" : "[FAIL]");
                        ImGui::SameLine();
                        ImGui::Text("%s: %s", r.name.c_str(), r.message.c_str());
                    }
                }
                ImGui::EndTabItem();
            }
            
            // --- TAB: PERFORMANCE ---
            if (ImGui::BeginTabItem("Performance")) {
                UpdatePerformanceGraph(deltaTime);
                ImGui::PlotLines("Frame Time (ms)", frameTimes, 100, frameOffset, "", 0.0f, 33.0f, ImVec2(0, 80));
                ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
                ImGui::Separator();
                ImGui::Text("Active Chunks: %zu", terrainManager.chunks.size());
                
                size_t totalTrees = 0, totalGrass = 0;
                for(auto const& [key, chunk] : terrainManager.chunks) {
                    totalTrees += chunk->treeTransforms.size();
                    totalGrass += chunk->grassTransforms.size();
                }
                ImGui::Text("Instanced Objects: %zu", totalTrees + totalGrass);
                ImGui::EndTabItem();
            }

            // --- TAB: TERRAIN ---
            if (ImGui::BeginTabItem("Terrain")) {
                if (ImGui::SliderFloat("Amplitude", &generator.amplitude, 0.0f, 200.0f)) { generator.UpdateSettings(); terrainManager.Clear(); }
                if (ImGui::SliderFloat("Frequency", &generator.frequency, 0.001f, 0.05f)) { generator.UpdateSettings(); terrainManager.Clear(); }
                if (ImGui::SliderInt("Octaves", &generator.octaves, 1, 8)) { generator.UpdateSettings(); terrainManager.Clear(); }
                
                ImGui::Separator();
                if (ImGui::Button("Randomize Seed & Regenerate")) {
                    generator.noise.SetSeed(rand());
                    terrainManager.Clear();
                }
                ImGui::EndTabItem();
            }

            // --- TAB: ENVIRONMENT ---
            if (ImGui::BeginTabItem("Environment")) {
                ImGui::SliderFloat("Time", &envManager.worldTime, 0.0f, 24.0f);
                ImGui::SliderFloat("Time Scale", &envManager.timeScale, 0.0f, 10.0f);
                ImGui::Checkbox("Pause Time", &envManager.isPaused);
                ImGui::Separator();
                ImGui::SliderFloat("Fog Density", &envManager.fogDensity, 0.0f, 0.05f);
                ImGui::SliderFloat("Water Level", &envManager.waterHeight, 0.0f, 20.0f);
                ImGui::EndTabItem();
            }

            // --- TAB: RENDERING ---
            if (ImGui::BeginTabItem("Rendering")) {
                ImGui::Checkbox("Wireframe Mode", &wireframe);
                ImGui::Checkbox("Cinematic Auto-Fly", &presManager.cinematicMode);
                ImGui::Separator();
                ImGui::Text("Post-Processing");
                ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);
                ImGui::SliderFloat("Bloom Strength", &bloomStrength, 0.0f, 2.0f);
                ImGui::ColorEdit3("Global Tint", glm::value_ptr(tint));
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

private:
    void UpdatePerformanceGraph(float deltaTime) {
        frameTimes[frameOffset] = deltaTime * 1000.0f;
        frameOffset = (frameOffset + 1) % 100;
    }
};
