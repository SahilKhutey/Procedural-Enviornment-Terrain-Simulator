#pragma once

#include "../renderer/Mesh.h"
#include <vector>

class Water {
public:
    Mesh* waterMesh;

    Water(float size = 2000.0f) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // Simple large quad for water
        vertices.push_back({glm::vec3(-size, 0, -size), glm::vec3(0, 1, 0), glm::vec3(0.1, 0.4, 0.8), glm::vec2(0, 0)});
        vertices.push_back({glm::vec3(size, 0, -size), glm::vec3(0, 1, 0), glm::vec3(0.1, 0.4, 0.8), glm::vec2(1, 0)});
        vertices.push_back({glm::vec3(size, 0, size), glm::vec3(0, 1, 0), glm::vec3(0.1, 0.4, 0.8), glm::vec2(1, 1)});
        vertices.push_back({glm::vec3(-size, 0, size), glm::vec3(0, 1, 0), glm::vec3(0.1, 0.4, 0.8), glm::vec2(0, 1)});

        indices = {0, 2, 1, 0, 3, 2};

        waterMesh = new Mesh(vertices, indices);
    }

    void Draw() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        waterMesh->Draw();
        glDisable(GL_BLEND);
    }

    ~Water() {
        delete waterMesh;
    }
};
