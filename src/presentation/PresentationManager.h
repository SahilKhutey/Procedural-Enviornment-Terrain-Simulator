#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../camera/Camera.h"

class PresentationManager {
public:
    bool cinematicMode = false;
    float cinematicSpeed = 0.5f;
    float cinematicHeight = 60.0f;
    float cinematicRadius = 150.0f;

    void Update(Camera& camera, float time, float deltaTime) {
        if (cinematicMode) {
            float angle = time * cinematicSpeed;
            glm::vec3 targetPos = glm::vec3(
                sin(angle) * cinematicRadius,
                cinematicHeight + sin(angle * 0.5f) * 10.0f,
                cos(angle) * cinematicRadius
            );

            // Smooth interpolation to position
            camera.Position = glm::mix(camera.Position, targetPos, deltaTime * 2.0f);
            
            // Always look at center
            glm::vec3 center = glm::vec3(0, 10.0f, 0);
            glm::vec3 direction = glm::normalize(center - camera.Position);
            
            // Calculate Yaw/Pitch from direction
            camera.Yaw = glm::degrees(atan2(direction.z, direction.x));
            camera.Pitch = glm::degrees(asin(direction.y));
            camera.updateCameraVectors();
        }
    }
};
