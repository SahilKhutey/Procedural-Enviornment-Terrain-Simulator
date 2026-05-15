# Procedural Environmental Terrain Simulator

PETS is a hybrid procedural environment project with two parallel implementations:

- A `Three.js` web simulator for fast iteration, biome previewing, and control-panel-driven experimentation
- A native `C++ / OpenGL` simulator scaffold for a fuller engine-style implementation

The current repo is best experienced through the web simulator because it launches quickly, exposes the procedural controls directly, and does not require a native compiler.

## What This Project Includes

- Procedural terrain driven by layered noise
- Day/night lighting and fog
- Water height and wave controls
- Biome presets with terrain, material, and ecosystem defaults
- Lightweight tree, rock, and fern variants for interactive previewing
- Asset-library metadata with source links for future real-model replacement
- Native engine scaffolding for terrain, water, sky, vegetation, GUI, and diagnostics

## Repository Structure

```text
Simulation-World/
├─ assets/
│  ├─ biomes/                  # biome pack JSON files
│  ├─ database/                # asset and web biome library manifests
│  └─ shaders/                 # GLSL shaders for native renderer
├─ src/                        # native C++ engine headers/modules
├─ engine.js                   # web simulator runtime
├─ index.html                  # web UI layout
├─ style.css                   # web simulator styling
├─ main.cpp                    # native simulator entrypoint
├─ LaunchWebSimulator.bat      # one-click web launcher
├─ LaunchSimulator.bat         # native build-and-run helper
└─ CMakeLists.txt              # native build configuration
```

## Quick Start

### Web Simulator

The web simulator is the recommended entrypoint.

1. Run [LaunchWebSimulator.bat](C:/Users/User/Documents/Simulation-World/LaunchWebSimulator.bat:1)
2. Wait for the local server to start
3. Open [http://127.0.0.1:8000/index.html](http://127.0.0.1:8000/index.html) if the browser does not open automatically

You can also start it manually:

```powershell
cd C:\Users\User\Documents\Simulation-World
python -m http.server 8000 --bind 127.0.0.1
```

Then open:

[http://127.0.0.1:8000/index.html](http://127.0.0.1:8000/index.html)

### Native Simulator

The native version requires a working C++ toolchain.

1. Install Visual Studio C++ tools or MinGW
2. Run [LaunchSimulator.bat](C:/Users/User/Documents/Simulation-World/LaunchSimulator.bat:1)

If no compiler is installed, the script will stop and show the setup steps.

## Control Panel Overview

### Terrain

- `Amplitude`: terrain height range
- `Complexity`: terrain frequency/noise scale
- `Octaves`: layered noise detail
- `Fog Density`: atmospheric falloff
- `Grass/Rock Color`: live terrain palette overrides
- `Regenerate Seed`: rebuild terrain distribution with a new random seed

### Biome

- `Biome Preset`: applies a full environment preset
- `Material Preset`: swaps color and water styling
- `Tree/Rock/Fern Model`: lightweight preview variants
- Each selector includes a note with source references for future downloadable replacements

### Water + Time

- `World Time`: sun position and lighting mood
- `Auto-Cycle Time`: animated day-night progression
- `Water Level`: baseline water plane elevation
- `Wave Strength / Wave Speed`: water movement tuning

### Ecosystem

- `Tree Density`
- `Rock Density`
- `Fern Density`
- `Distribution Min/Max Height`
- `Distribution Slope Limit`
- `Scale Min/Max`

### Render

- `Exposure`
- `Bloom`
- `Wireframe`

## Controls

### Camera

- `Mouse`: look around after entering simulator mode
- `W A S D`: move horizontally
- `Enter Simulator`: lock pointer and start immersive navigation
- `Esc`: exit pointer lock

## Asset Strategy

The current web experience intentionally uses lightweight procedural proxy models instead of full downloaded `.glb` assets. This keeps startup and interaction responsive while still exposing asset categories and biome logic.

The manifest at [web-biome-library.json](C:/Users/User/Documents/Simulation-World/assets/database/web-biome-library.json:1) stores:

- biome presets
- tree variants
- rock variants
- fern variants
- material presets
- source references for future real asset downloads

## Documentation

For the deeper technical explanation, see:

- [docs/ARCHITECTURE.md](C:/Users/User/Documents/Simulation-World/docs/ARCHITECTURE.md:1)
- [docs/WEB_SIMULATOR_GUIDE.md](C:/Users/User/Documents/Simulation-World/docs/WEB_SIMULATOR_GUIDE.md:1)

## Current Limitations

- The native C++ side is scaffolded but not fully validated in this environment because a local compiler was not available during development
- The web simulator uses procedural proxy assets rather than imported real 3D models
- Asset source links are documented, but automated downloading/import pipelines are not implemented yet
- The web simulator is optimized for responsiveness, not physically accurate terrain sampling

## Suggested Next Steps

- Replace proxy asset variants with real `.glb` packs
- Add asynchronous chunk-based ecosystem streaming
- Add save/load for user-created biome presets
- Add texture-based materials for terrain and water
- Complete and validate the native C++ build pipeline on a machine with Visual Studio C++ tools installed
