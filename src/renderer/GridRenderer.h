#pragma once

#include "Mesh.h"
#include "Shader.h"
#include <vector>

class GridRenderer {
public:
    Mesh* gridMesh;
    
    GridRenderer(int size = 100, int spacing = 1) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (int i = -size; i <= size; i += spacing) {
            // Horizontal lines
            vertices.push_back({glm::vec3(i, 0, -size), glm::vec3(0, 1, 0), glm::vec3(0.35f), glm::vec2(0, 0)});
            vertices.push_back({glm::vec3(i, 0, size), glm::vec3(0, 1, 0), glm::vec3(0.35f), glm::vec2(0, 0)});
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 1);

            // Vertical lines
            vertices.push_back({glm::vec3(-size, 0, i), glm::vec3(0, 1, 0), glm::vec3(0.35f), glm::vec2(0, 0)});
            vertices.push_back({glm::vec3(size, 0, i), glm::vec3(0, 1, 0), glm::vec3(0.35f), glm::vec2(0, 0)});
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 1);
        }

        gridMesh = new Mesh(vertices, indices);
    }

    void Draw(Shader& shader) {
        shader.use();
        gridMesh->DrawLines();
    }

    ~GridRenderer() {
        delete gridMesh;
    }
};
