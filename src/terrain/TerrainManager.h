#pragma once

#include "TerrainChunk.h"
#include <map>
#include <string>
#include <cmath>

class TerrainManager {
public:
    int chunkSize = 64;
    int renderDistance = 4;
    std::map<std::string, TerrainChunk*> chunks;
    TerrainGenerator& generator;

    TerrainManager(TerrainGenerator& gen) : generator(gen) {}

    void Update(glm::vec3 cameraPos) {
        int currentX = (int)std::floor(cameraPos.x / (chunkSize - 1));
        int currentZ = (int)std::floor(cameraPos.z / (chunkSize - 1));

        // Load new chunks
        for (int x = currentX - renderDistance; x <= currentX + renderDistance; x++) {
            for (int z = currentZ - renderDistance; z <= currentZ + renderDistance; z++) {
                std::string key = std::to_string(x) + "," + std::to_string(z);
                if (chunks.find(key) == chunks.end()) {
                    chunks[key] = new TerrainChunk(x, z, chunkSize, generator);
                }
            }
        }

        // Unload far chunks
        for (auto it = chunks.begin(); it != chunks.end();) {
            int cx = it->second->x;
            int cz = it->second->z;
            if (std::abs(cx - currentX) > renderDistance + 1 || std::abs(cz - currentZ) > renderDistance + 1) {
                delete it->second;
                it = chunks.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Draw(Shader& shader) {
        shader.use();
        for (auto const& [key, chunk] : chunks) {
            chunk->Draw();
        }
    }

    void Clear() {
        for (auto const& [key, chunk] : chunks) {
            delete chunk;
        }
        chunks.clear();
    }

    ~TerrainManager() {
        Clear();
    }
};
