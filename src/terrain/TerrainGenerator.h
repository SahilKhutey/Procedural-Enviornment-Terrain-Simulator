#pragma once

#include <FastNoiseLite.h>
#include <glm/glm.hpp>
#include <vector>
#include "../database/AssetDatabase.h"

class TerrainGenerator {
public:
    FastNoiseLite noise;
    float frequency = 0.01f;
    float amplitude = 25.0f;
    int octaves = 4;
    float lacunarity = 2.0f;
    float gain = 0.5f;
    AssetDatabase* database = nullptr;

    // Biome Colors
    glm::vec3 sand = glm::vec3(0.76f, 0.70f, 0.50f);
    glm::vec3 grass = glm::vec3(0.30f, 0.50f, 0.20f);
    glm::vec3 rock = glm::vec3(0.40f, 0.40f, 0.40f);
    glm::vec3 snow = glm::vec3(0.95f, 0.95f, 0.95f);
    glm::vec3 water_deep = glm::vec3(0.10f, 0.20f, 0.40f);

    TerrainGenerator() {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    }

    void UpdateSettings() {
        noise.SetFrequency(frequency);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(octaves);
        noise.SetFractalLacunarity(lacunarity);
        noise.SetFractalGain(gain);
    }

    float GetHeight(float x, float z) {
        return noise.GetNoise(x, z) * amplitude;
    }

    glm::vec3 GetBiomeColor(float height, glm::vec3 normal) {
        if (!database) return glm::vec3(0.5f);

        // Find biome from database
        for (auto const& [name, biome] : database->biomes) {
            if (height >= biome.minHeight && height < biome.maxHeight) {
                return biome.terrainColor;
            }
        }
        
        return glm::vec3(1.0f); // Default white (snow)
    }
};
