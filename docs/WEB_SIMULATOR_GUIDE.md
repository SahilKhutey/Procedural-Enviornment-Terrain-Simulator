# Web Simulator Guide

This guide focuses on how to use and extend the browser-based PETS simulator.

## Launching

Use [LaunchWebSimulator.bat](C:/Users/User/Documents/Simulation-World/LaunchWebSimulator.bat:1) for the easiest workflow.

It:

1. starts a local Python server
2. opens the simulator URL
3. leaves the server window open until you close it

If the browser does not open automatically, visit:

[http://127.0.0.1:8000/index.html](http://127.0.0.1:8000/index.html)

## Why the Server Is Required

The web simulator loads ES modules and JSON manifests. Opening `index.html` directly from the filesystem can cause loading failures, so a local HTTP server is used instead.

## Tab-by-Tab Usage

### Terrain Tab

Use this tab to shape the land itself.

- `Amplitude` changes how tall the terrain becomes
- `Complexity` changes how often the hills and valleys repeat
- `Octaves` increases procedural detail
- `Fog Density` changes the depth fade
- `Grass/Rock Color` lets you override the active material palette
- `Regenerate Seed` keeps the same tuning but produces a different distribution pattern

These controls trigger terrain or ecosystem updates.

### Biome Tab

Use this tab to switch higher-level environmental themes.

- `Biome Preset` changes terrain, water, vegetation density, and default assets together
- `Material Preset` changes the look of grass, rock, water, and fog
- `Tree Model`, `Rock Model`, and `Fern Model` switch between lightweight preview variants

Each selection also shows a note with:

- description
- source link
- license/source-pack note

This helps track future asset-replacement candidates.

### Water + Time Tab

- `World Time` rotates the sun and changes atmosphere
- `Auto-Cycle Time` animates the day/night loop
- `Water Level` moves the base water plane
- `Wave Strength` controls water motion amplitude
- `Wave Speed` controls how quickly the water oscillates

### Ecosystem Tab

This tab controls how environment props are distributed.

- `Tree Density`
- `Rock Density`
- `Fern Density`
- `Distribution Min Height`
- `Distribution Max Height`
- `Distribution Slope Limit`
- `Scale Min`
- `Scale Max`

These values strongly affect both appearance and performance.

## Performance Notes

If the browser becomes sluggish:

- reduce tree density first
- reduce fern density next
- keep octaves moderate
- avoid repeated rapid dragging of many sliders at once

The current implementation is much more responsive than the earlier raycast-heavy version, but high densities still cost CPU and GPU time.

## Asset Replacement Workflow

Right now the web simulator uses procedural proxy geometry. To replace these with real assets later:

1. pick a source from [web-biome-library.json](C:/Users/User/Documents/Simulation-World/assets/database/web-biome-library.json:1)
2. download small `.glb` assets
3. place them under a dedicated asset folder such as `assets/models/`
4. load them with a runtime loader
5. preserve the same category/variant IDs so the panel wiring stays stable

Suggested folders for future expansion:

```text
assets/
├─ models/
│  ├─ trees/
│  ├─ rocks/
│  └─ ferns/
└─ textures/
   ├─ terrain/
   └─ water/
```

## Important Files

- [engine.js](C:/Users/User/Documents/Simulation-World/engine.js:1): main runtime logic
- [index.html](C:/Users/User/Documents/Simulation-World/index.html:1): panel structure
- [style.css](C:/Users/User/Documents/Simulation-World/style.css:1): layout and styling
- [web-biome-library.json](C:/Users/User/Documents/Simulation-World/assets/database/web-biome-library.json:1): data-driven preset and asset metadata

## Troubleshooting

### “This site can’t be reached”

The local server is not running. Start it with [LaunchWebSimulator.bat](C:/Users/User/Documents/Simulation-World/LaunchWebSimulator.bat:1).

### “Page unresponsive”

This usually means the ecosystem settings are too aggressive for the current machine or browser session.

Try:

- lowering tree density
- lowering fern density
- reloading the page after reducing settings

### Controls change but the scene does not

Some controls affect only visuals, while others rebuild the ecosystem. If a control seems delayed, wait briefly for the scheduled rebuild to finish.
