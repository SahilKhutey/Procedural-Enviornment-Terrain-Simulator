#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

class EnvironmentManager {
public:
    // Time System
    float worldTime = 12.0f; // 0.0 - 24.0
    float timeScale = 1.0f;
    bool isPaused = false;

    // Lighting State
    glm::vec3 sunDir;
    glm::vec3 moonDir;
    glm::vec3 sunColor;
    glm::vec3 ambientColor;
    glm::vec3 skyColorTop;
    glm::vec3 skyColorBottom;
    glm::vec3 fogColor;
    float fogDensity = 0.015f;

    // Water Settings
    float waterHeight = 4.0f;
    float waveSpeed = 1.0f;
    float waveStrength = 0.05f;

    void Update(float deltaTime) {
        if (!isPaused) {
            worldTime += deltaTime * (timeScale * 0.1f);
            // Smooth wrap logic
            if (worldTime >= 24.0f) worldTime -= 24.0f;
            if (worldTime < 0.0f) worldTime += 24.0f;
        }

        UpdateLighting();
    }

private:
    void UpdateLighting() {
        // Sun Angle: 0 = Noon, 6 = Sunset, 12 = Midnight, 18 = Sunrise
        // Adjusting offset so worldTime 12.0 is high noon
        float sunAngle = ((worldTime - 12.0f) / 24.0f) * 2.0f * 3.14159f;
        
        // Rotating on Z-Y plane for a natural arc
        sunDir = glm::normalize(glm::vec3(0.0f, cos(sunAngle), sin(sunAngle)));
        moonDir = -sunDir;

        float dayFactor = glm::clamp(sunDir.y, -1.0f, 1.0f);
        
        // --- Smooth Interpolation Logic ---
        
        // Noon (dayFactor ~ 1.0)
        // Sunset/Sunrise (dayFactor ~ 0.0)
        // Night (dayFactor ~ -1.0)

        if (dayFactor > 0.0f) { // Day / Sunset / Sunrise
            float t = dayFactor;
            sunColor = glm::mix(glm::vec3(1.0f, 0.4f, 0.1f), glm::vec3(1.0f, 0.9f, 0.8f), t);
            ambientColor = glm::mix(glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.4f, 0.5f, 0.6f), t);
            skyColorTop = glm::mix(glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.2f, 0.4f, 0.8f), t);
            skyColorBottom = glm::mix(glm::vec3(1.0f, 0.5f, 0.2f), glm::vec3(0.6f, 0.8f, 1.0f), t);
        } else { // Night
            float t = std::abs(dayFactor);
            sunColor = glm::vec3(0.1f, 0.1f, 0.2f); // Dim moon
            ambientColor = glm::vec3(0.02f, 0.02f, 0.05f);
            skyColorTop = glm::vec3(0.02f, 0.02f, 0.05f);
            skyColorBottom = glm::vec3(0.05f, 0.05f, 0.1f);
        }
        
        fogColor = skyColorBottom;
    }
};
