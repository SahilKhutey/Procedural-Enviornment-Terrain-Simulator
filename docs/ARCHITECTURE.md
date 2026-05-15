# Architecture Notes

This document explains how the current PETS repository is organized and how the major systems fit together.

## High-Level Design

The repository currently contains two simulator tracks.

### 1. Web Simulator

The web simulator is implemented with:

- [index.html](C:/Users/User/Documents/Simulation-World/index.html:1)
- [style.css](C:/Users/User/Documents/Simulation-World/style.css:1)
- [engine.js](C:/Users/User/Documents/Simulation-World/engine.js:1)

Its purpose is rapid prototyping:

- fast launch
- interactive control panel
- lightweight biome experimentation
- visual validation without native compilation

### 2. Native Simulator Scaffold

The native simulator is structured around:

- [main.cpp](C:/Users/User/Documents/Simulation-World/main.cpp:1)
- [src/terrain](C:/Users/User/Documents/Simulation-World/src/terrain/TerrainGenerator.h:1)
- [src/environment](C:/Users/User/Documents/Simulation-World/src/environment/EnvironmentManager.h:1)
- [src/ecosystem](C:/Users/User/Documents/Simulation-World/src/ecosystem/Vegetation.h:1)
- [src/gui](C:/Users/User/Documents/Simulation-World/src/gui/GUIManager.h:1)

Its purpose is engine architecture, rendering-system organization, and future migration toward a fuller graphics implementation.

## Web Runtime Flow

The web app boots in this order:

1. Construct renderer, scene, camera, telemetry handles, and default params
2. Load [web-biome-library.json](C:/Users/User/Documents/Simulation-World/assets/database/web-biome-library.json:1)
3. Initialize terrain, water, lighting, and materials
4. Populate the control panel
5. Apply the default biome preset
6. Schedule an ecosystem rebuild
7. Start the animation loop

## Terrain System

The terrain is generated procedurally in the shader and mirrored approximately in JavaScript for ecosystem placement.

### Shader Side

In [engine.js](C:/Users/User/Documents/Simulation-World/engine.js:107), the terrain material uses:

- `uAmplitude`
- `uFrequency`
- `uOctaves`
- `uSeed`
- `uMode`

These are used to build an FBM-style heightfield directly on the GPU.

### Placement Side

To avoid the browser freezing during startup, ecosystem placement no longer relies on repeated mesh raycasts. Instead, JavaScript samples the same noise-inspired height function directly using:

- `hash2`
- `noise2`
- `fbm2`
- `sampleTerrainHeight`
- `sampleTerrainNormal`

This is faster and keeps startup responsive.

## Ecosystem System

The web ecosystem is built from lightweight instanced proxy geometry.

### Categories

- Trees
- Rocks
- Ferns

### Variant Model

Each category has three variants defined procedurally in [engine.js](C:/Users/User/Documents/Simulation-World/engine.js:598):

- Trees: `pine`, `oak`, `dead`
- Rocks: `pebble`, `mossy`, `cliff`
- Ferns: `tuft`, `fan`, `bush`

Each variant is assembled from simple primitive geometry such as:

- cones
- cylinders
- dodecahedrons
- boxes
- octahedrons

This keeps the app light enough for live regeneration.

### Placement Rules

Each ecosystem spawn checks:

- min height
- max height
- slope threshold
- water offset
- scale range

This data comes from live panel values and biome presets.

## Biome and Asset Manifest

The main web manifest is [assets/database/web-biome-library.json](C:/Users/User/Documents/Simulation-World/assets/database/web-biome-library.json:1).

It stores:

- biome presets
- ecosystem defaults
- water defaults
- terrain defaults
- material presets
- variant metadata
- external source references

This lets the control panel remain data-driven instead of hardcoding every option into the HTML.

## UI Architecture

The control panel is split into these tabs:

- Terrain
- Biome
- Water + Time
- Ecosystem
- Render

Each tab maps directly to runtime parameter groups in [engine.js](C:/Users/User/Documents/Simulation-World/engine.js:39).

## Responsiveness Strategy

The page became unresponsive when startup tried to do too much synchronous work. The current design reduces that risk by:

- using instanced rendering
- using primitive-based proxy assets
- deferring ecosystem regeneration with `requestAnimationFrame`
- sampling terrain procedurally instead of raycasting against dense geometry

## Native Side Overview

The native C++ side is more modular than the web prototype and is arranged by engine concern:

- camera
- terrain
- renderer
- ecosystem
- environment
- GUI
- presentation
- debugging
- database

Examples:

- [TerrainManager.h](C:/Users/User/Documents/Simulation-World/src/terrain/TerrainManager.h:1) manages chunk lifetime
- [TerrainChunk.h](C:/Users/User/Documents/Simulation-World/src/terrain/TerrainChunk.h:1) generates mesh data and spawn transforms
- [EnvironmentManager.h](C:/Users/User/Documents/Simulation-World/src/environment/EnvironmentManager.h:1) handles time and lighting
- [GUIManager.h](C:/Users/User/Documents/Simulation-World/src/gui/GUIManager.h:1) exposes debug and tweak controls

## Build System

[CMakeLists.txt](C:/Users/User/Documents/Simulation-World/CMakeLists.txt:1) uses `FetchContent` for:

- GLFW
- GLAD
- GLM
- nlohmann/json
- ImGui
- FastNoiseLite

This is intended to make the native setup reproducible once a compiler is present.

## Known Gaps

- The web terrain sampler is intentionally approximate rather than guaranteed bit-identical to the shader
- The web asset library stores source references but does not yet import external model files automatically
- The native side still needs full compile/runtime validation on a proper C++ toolchain
