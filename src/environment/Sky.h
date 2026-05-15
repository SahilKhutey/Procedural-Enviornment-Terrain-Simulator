#pragma once

#include "../renderer/Mesh.h"
#include <vector>

class Sky {
public:
    Mesh* skyMesh;

    Sky(float radius = 1000.0f) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        int stacks = 20;
        int sectors = 20;
        float PI = 3.14159f;

        for (int i = 0; i <= stacks; ++i) {
            float stackAngle = PI / 2 - i * PI / stacks;
            float y = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; ++j) {
                float sectorAngle = j * 2 * PI / sectors;
                float x = radius * cosf(stackAngle) * cosf(sectorAngle);
                float z = radius * cosf(stackAngle) * sinf(sectorAngle);

                Vertex v;
                v.Position = glm::vec3(x, y, z);
                v.Normal = glm::normalize(-v.Position); // Inverted for sky dome
                v.Color = glm::vec3(1.0f);
                v.TexCoords = glm::vec2((float)j / sectors, (float)i / stacks);
                vertices.push_back(v);
            }
        }

        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < sectors; ++j) {
                unsigned int first = (i * (sectors + 1)) + j;
                unsigned int second = first + sectors + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        skyMesh = new Mesh(vertices, indices);
    }

    void Draw() {
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE); // Render sky behind everything
        skyMesh->Draw();
        glDepthMask(GL_TRUE);
    }

    ~Sky() {
        delete skyMesh;
    }
};
