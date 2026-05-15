#pragma once

#include "../renderer/Mesh.h"
#include <glm/gtc/matrix_transform.hpp>

class InstancedMesh : public Mesh {
public:
    unsigned int instanceVBO;
    int instanceCount = 0;

    InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) 
        : Mesh(vertices, indices) {
        setupInstanceBuffer();
    }

    void UpdateInstances(const std::vector<glm::mat4>& models) {
        instanceCount = static_cast<int>(models.size());
        if (instanceCount == 0) return;

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), &models[0], GL_DYNAMIC_DRAW);
    }

    void DrawInstanced() {
        if (instanceCount == 0) return;
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, instanceCount);
        glBindVertexArray(0);
    }

private:
    void setupInstanceBuffer() {
        glGenBuffers(1, &instanceVBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // Instance Matrix (4 vec4s)
        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(4); 
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(5); 
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(6); 
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(7); 
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);

        glBindVertexArray(0);
    }
};
