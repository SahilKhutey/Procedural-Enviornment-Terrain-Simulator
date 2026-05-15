#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

struct AssetDefinition {
    std::string name;
    float minScale;
    float maxScale;
    float spawnProbability;
    float minHeight;
    float maxHeight;
    float maxSlope;
};

struct BiomeDefinition {
    std::string name;
    glm::vec3 terrainColor;
    float minHeight;
    float maxHeight;
    std::vector<std::string> allowedAssets;
};

class AssetDatabase {
public:
    std::map<std::string, AssetDefinition> assets;
    std::map<std::string, BiomeDefinition> biomes;

    void LoadDatabase(const std::string& assetPath, const std::string& biomePath) {
        LoadAssets(assetPath);
        LoadBiomes(biomePath);
    }

private:
    void LoadAssets(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return;
        
        json j;
        file >> j;
        for (auto& item : j) {
            AssetDefinition ad;
            ad.name = item["name"];
            ad.minScale = item["minScale"];
            ad.maxScale = item["maxScale"];
            ad.spawnProbability = item["spawnProbability"];
            ad.minHeight = item["minHeight"];
            ad.maxHeight = item["maxHeight"];
            ad.maxSlope = item["maxSlope"];
            assets[ad.name] = ad;
        }
    }

    void LoadBiomes(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return;

        json j;
        file >> j;
        for (auto& item : j) {
            BiomeDefinition bd;
            bd.name = item["name"];
            bd.terrainColor = glm::vec3(item["color"][0], item["color"][1], item["color"][2]);
            bd.minHeight = item["minHeight"];
            bd.maxHeight = item["maxHeight"];
            for (auto& a : item["assets"]) bd.allowedAssets.push_back(a);
            biomes[bd.name] = bd;
        }
    }
};
