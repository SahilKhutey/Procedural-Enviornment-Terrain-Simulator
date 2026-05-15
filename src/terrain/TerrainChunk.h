#pragma once

#include "../renderer/Mesh.h"
#include "TerrainGenerator.h"
#include <vector>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>

class TerrainChunk {
public:
    int x, z;
    int size;
    Mesh* mesh;

    TerrainChunk(int chunkX, int chunkZ, int chunkSize, TerrainGenerator& generator) {
        x = chunkX;
        z = chunkZ;
        size = chunkSize;
        Generate(generator);
    }

    void Generate(TerrainGenerator& generator) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float offsetX = x * (size - 1);
        float offsetZ = z * (size - 1);

        // 1. Generate Vertices & Heights
        for (int row = 0; row < size; row++) {
            for (int col = 0; col < size; col++) {
                float worldX = col + offsetX;
                float worldZ = row + offsetZ;
                float height = generator.GetHeight(worldX, worldZ);

                Vertex v;
                v.Position = glm::vec3(worldX, height, worldZ);
                v.Normal = glm::vec3(0, 1, 0); // Placeholder
                v.TexCoords = glm::vec2((float)col / size, (float)row / size);
                vertices.push_back(v);
            }
        }

        // 2. Generate Indices
        for (int row = 0; row < size - 1; row++) {
            for (int col = 0; col < size - 1; col++) {
                unsigned int topLeft = row * size + col;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (row + 1) * size + col;
                unsigned int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        // 3. Calculate Normals & Colors
        CalculateNormals(vertices, indices);
        for (auto& v : vertices) {
            v.Color = generator.GetBiomeColor(v.Position.y, v.Normal);
        }

        mesh = new Mesh(vertices, indices);

        // --- Ecosystem Spawning (Data-Driven) ---
        if (generator.database) {
            for (size_t i = 0; i < vertices.size(); i += 4) {
                Vertex& v = vertices[i];
                float slope = 1.0f - glm::dot(v.Normal, glm::vec3(0, 1, 0));

                for (auto const& [name, asset] : generator.database->assets) {
                    if (v.Position.y > asset.minHeight && v.Position.y < asset.maxHeight && slope < asset.maxSlope) {
                        if ((rand() % 1000) / 1000.0f < asset.spawnProbability) {
                            float scale = asset.minScale + (rand() % 100) / 100.0f * (asset.maxScale - asset.minScale);
                            float rotation = (rand() % 360) * (3.14159f / 180.0f);

                            glm::mat4 model = glm::translate(glm::mat4(1.0f), v.Position);
                            model = glm::rotate(model, rotation, glm::vec3(0, 1, 0));
                            model = glm::scale(model, glm::vec3(scale));
                            
                            if (name == "Tree") treeTransforms.push_back(model);
                            if (name == "Grass") grassTransforms.push_back(model);
                        }
                    }
                }
            }
        }
    }

    std::vector<glm::mat4> treeTransforms;
    std::vector<glm::mat4> grassTransforms;

    void Draw() {
        mesh->Draw();
    }

    ~TerrainChunk() {
        delete mesh;
    }

private:
    void CalculateNormals(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        for (size_t i = 0; i < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i + 1];
            unsigned int i2 = indices[i + 2];

            glm::vec3 v0 = vertices[i0].Position;
            glm::vec3 v1 = vertices[i1].Position;
            glm::vec3 v2 = vertices[i2].Position;

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

            vertices[i0].Normal += normal;
            vertices[i1].Normal += normal;
            vertices[i2].Normal += normal;
        }

        for (auto& v : vertices) {
            v.Normal = glm::normalize(v.Normal);
        }
    }
};
