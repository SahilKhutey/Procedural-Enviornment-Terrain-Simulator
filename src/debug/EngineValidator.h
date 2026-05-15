#pragma once

#include "TestRunner.h"
#include "../terrain/TerrainGenerator.h"
#include "../terrain/TerrainManager.h"
#include "../camera/Camera.h"

class EngineValidator {
public:
    static void RunAllTests(TestRunner& tr, TerrainGenerator& gen, TerrainManager& tm, Camera& cam) {
        tr.Clear();

        // 1. Terrain Height Checks
        float h = gen.noise.GetNoise(0.0f, 0.0f) * gen.amplitude;
        tr.Assert(h >= -gen.amplitude && h <= gen.amplitude, 
                  "Terrain Range", "Height is within mathematical noise bounds.");

        // 2. Noise Determinism
        float h1 = gen.noise.GetNoise(50.0f, 50.0f);
        float h2 = gen.noise.GetNoise(50.0f, 50.0f);
        tr.Assert(h1 == h2, "Noise Determinism", "Noise function returns consistent values for same coords.");

        // 3. Chunk Integrity
        tr.Assert(!tm.chunks.empty(), "Chunk Presence", "The world has at least one active chunk.");
        
        // 4. Ecosystem Safety
        size_t totalInst = 0;
        for(auto const& [key, chunk] : tm.chunks) {
            totalInst += chunk->treeTransforms.size() + chunk->grassTransforms.size();
        }
        tr.Assert(totalInst < 100000, "Memory Safety", "Instance count is within safe VRAM limits.");

        // 5. Camera Projection
        tr.Assert(cam.Zoom > 0.0f && cam.Zoom < 180.0f, "Camera Safety", "Projection FOV is in a valid range.");

        // 6. Terrain Smoothness (Edge Case)
        float ha = gen.noise.GetNoise(10.0f, 10.0f);
        float hb = gen.noise.GetNoise(10.1f, 10.1f);
        tr.Assert(std::abs(ha - hb) < 1.0f, "Terrain Smoothness", "No sharp discontinuities detected in heightmap.");
    }
};
