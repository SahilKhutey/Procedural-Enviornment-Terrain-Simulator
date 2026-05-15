#pragma once

#include "InstancedMesh.h"
#include <vector>
#include <cmath>

class VegetationGenerator {
public:
    static InstancedMesh* CreateTree() {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // --- Simple Trunk (Cylinder) ---
        glm::vec3 trunkCol = glm::vec3(0.4f, 0.25f, 0.1f);
        for (int i = 0; i < 8; ++i) {
            float angle = i * 3.14159f * 2.0f / 8.0f;
            vertices.push_back({glm::vec3(cos(angle) * 0.2f, 0, sin(angle) * 0.2f), glm::vec3(0, 1, 0), trunkCol, glm::vec2(0)});
            vertices.push_back({glm::vec3(cos(angle) * 0.2f, 2, sin(angle) * 0.2f), glm::vec3(0, 1, 0), trunkCol, glm::vec2(0)});
        }
        // Simplified indices for a cylinder... (just a placeholder logic)
        for (int i = 0; i < 14; i+=2) {
            indices.push_back(i); indices.push_back(i+1); indices.push_back(i+2);
            indices.push_back(i+1); indices.push_back(i+3); indices.push_back(i+2);
        }

        // --- Simple Canopy (Cone) ---
        glm::vec3 leafCol = glm::vec3(0.1f, 0.5f, 0.2f);
        unsigned int start = vertices.size();
        vertices.push_back({glm::vec3(0, 5, 0), glm::vec3(0, 1, 0), leafCol, glm::vec2(0)}); // Top
        for (int i = 0; i < 8; ++i) {
            float angle = i * 3.14159f * 2.0f / 8.0f;
            vertices.push_back({glm::vec3(cos(angle) * 1.5f, 1.5f, sin(angle) * 1.5f), glm::vec3(0, 1, 0), leafCol, glm::vec2(0)});
            indices.push_back(start);
            indices.push_back(start + 1 + i);
            indices.push_back(start + 1 + (i + 1) % 8);
        }

        return new InstancedMesh(vertices, indices);
    }

    static InstancedMesh* CreateGrass() {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        glm::vec3 grassCol = glm::vec3(0.4f, 0.7f, 0.2f);

        // Plane 1
        vertices.push_back({glm::vec3(-0.5, 0, 0), glm::vec3(0, 1, 0), grassCol, glm::vec2(0, 0)});
        vertices.push_back({glm::vec3(0.5, 0, 0), glm::vec3(0, 1, 0), grassCol, glm::vec2(1, 0)});
        vertices.push_back({glm::vec3(0.5, 1, 0), glm::vec3(0, 1, 0), grassCol, glm::vec2(1, 1)});
        vertices.push_back({glm::vec3(-0.5, 1, 0), glm::vec3(0, 1, 0), grassCol, glm::vec2(0, 1)});
        indices.push_back(0); indices.push_back(1); indices.push_back(2);
        indices.push_back(0); indices.push_back(2); indices.push_back(3);

        // Plane 2 (Rotated 90)
        vertices.push_back({glm::vec3(0, 0, -0.5), glm::vec3(0, 1, 0), grassCol, glm::vec2(0, 0)});
        vertices.push_back({glm::vec3(0, 0, 0.5), glm::vec3(0, 1, 0), grassCol, glm::vec2(1, 0)});
        vertices.push_back({glm::vec3(0, 1, 0.5), glm::vec3(0, 1, 0), grassCol, glm::vec2(1, 1)});
        vertices.push_back({glm::vec3(0, 1, -0.5), glm::vec3(0, 1, 0), grassCol, glm::vec2(0, 1)});
        indices.push_back(4); indices.push_back(5); indices.push_back(6);
        indices.push_back(4); indices.push_back(6); indices.push_back(7);

        return new InstancedMesh(vertices, indices);
    }
};
