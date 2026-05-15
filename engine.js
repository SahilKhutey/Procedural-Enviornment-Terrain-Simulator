import * as THREE from 'three';

class PETSEngine {
    constructor() {
        this.canvas = document.getElementById('engine-viewport');
        this.uiRoot = document.getElementById('ui-root');
        this.renderer = new THREE.WebGLRenderer({
            canvas: this.canvas,
            antialias: true,
            powerPreference: 'high-performance'
        });
        this.renderer.setSize(window.innerWidth, window.innerHeight);
        this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        this.renderer.toneMapping = THREE.ACESFilmicToneMapping;
        this.renderer.shadowMap.enabled = true;

        this.scene = new THREE.Scene();
        this.camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 1, 30000);
        this.camera.position.set(1500, 1000, 1500);

        this.clock = new THREE.Clock();
        this.worldSize = 15000;
        this.telemetry = {
            fps: document.getElementById('stat-fps'),
            instances: document.getElementById('stat-instances'),
            time: document.getElementById('stat-time')
        };
        this.cam = {
            pos: new THREE.Vector3(1500, 1000, 1500),
            front: new THREE.Vector3(0, 0, -1),
            up: new THREE.Vector3(0, 1, 0),
            right: new THREE.Vector3(1, 0, 0),
            worldUp: new THREE.Vector3(0, 1, 0),
            yaw: -45,
            pitch: -25,
            speed: 18,
            sensitivity: 0.15,
            damping: 0.1,
            keys: {},
            velocity: new THREE.Vector3()
        };

        this.params = {
            activeBiome: 'plains',
            selectedTreeVariant: 'pine',
            selectedRockVariant: 'pebble',
            selectedFernVariant: 'tuft',
            selectedMaterialPreset: 'meadow',
            time: 12,
            fog: 0.0001,
            amplitude: 45,
            frequency: 0.015,
            seed: 123,
            octaves: 6,
            mode: 0,
            autoCycle: false,
            bloom: 0.8,
            exposure: 1.2,
            wireframe: false,
            waterLevel: 10,
            waveStrength: 1.5,
            waveSpeed: 0.8,
            grassColor: '#2d5a27',
            rockColor: '#4a4a4a',
            waterColor: '#3c9fd8',
            waterOpacity: 0.5,
            treeDensity: 900,
            rockDensity: 320,
            fernDensity: 1500,
            distributionMinHeight: 4,
            distributionMaxHeight: 120,
            distributionSlopeLimit: 0.55,
            vegetationScaleMin: 0.8,
            vegetationScaleMax: 1.3
        };

        this.biome = {
            grass: new THREE.Color(this.params.grassColor),
            rock: new THREE.Color(this.params.rockColor),
            fog: new THREE.Color(0x95cfff),
            water: new THREE.Color(this.params.waterColor)
        };

        this.library = null;
        this.ecosystem = {};
        this.boot();
    }

    async boot() {
        this.library = await this.loadLibrary();
        this.initScene();
        this.populateControls();
        this.applyBiomePreset(this.params.activeBiome);
        this.setupEvents();
        this.setupEcosystem();
        this.animate();
    }

    async loadLibrary() {
        try {
            const response = await fetch('assets/database/web-biome-library.json');
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            return await response.json();
        } catch (error) {
            console.warn('PETS: Failed to load web biome library, using fallback library.', error);
            return {
                biomes: [],
                treeVariants: [],
                rockVariants: [],
                fernVariants: [],
                materialPresets: []
            };
        }
    }

    get noiseSource() {
        return `
            uniform float uSeed; uniform float uAmplitude; uniform float uFrequency; uniform float uOctaves; uniform float uMode;
            float hash(vec2 p) { vec3 p3 = fract(vec3(p.xyx) * .1031); p3 += dot(p3, p3.yzx + 33.33); return fract((p3.x + p3.y) * p3.z + uSeed); }
            float noise(vec2 p) { vec2 i = floor(p); vec2 f = fract(p); vec2 u = f*f*(3.0-2.0*f); return mix(mix(hash(i+vec2(0,0)), hash(i+vec2(1,0)), u.x), mix(hash(i+vec2(0,1)), hash(i+vec2(1,1)), u.x), u.y); }
            float fbm(vec2 p) {
                float v = 0.0; float a = 0.5;
                for (int i = 0; i < 8; i++) {
                    if (float(i) >= uOctaves) break;
                    float n = noise(p);
                    if (uMode > 0.5) n = 1.0 - abs(n * 2.0 - 1.0);
                    v += a * n; p *= 2.0; a *= 0.5;
                }
                return v;
            }
        `;
    }

    initScene() {
        this.scene.add(new THREE.AmbientLight(0xffffff, 0.6));
        this.scene.add(new THREE.HemisphereLight(0x95cfff, 0x2d5a27, 0.8));

        this.sun = new THREE.DirectionalLight(0xfff5ee, 1.5);
        this.sun.position.set(1000, 1000, 1000);
        this.scene.add(this.sun);

        this.sunMesh = new THREE.Mesh(
            new THREE.SphereGeometry(200),
            new THREE.MeshBasicMaterial({ color: 0xffffee, fog: false })
        );
        this.scene.add(this.sunMesh);

        const terrainGeo = new THREE.PlaneGeometry(this.worldSize, this.worldSize, 300, 300);
        terrainGeo.rotateX(-Math.PI / 2);

        this.terrainMat = new THREE.ShaderMaterial({
            uniforms: {
                uSunPos: { value: new THREE.Vector3() },
                uAmplitude: { value: this.params.amplitude },
                uFrequency: { value: this.params.frequency },
                uFogColor: { value: this.biome.fog.clone() },
                uFogDensity: { value: this.params.fog },
                uSeed: { value: this.params.seed },
                uOctaves: { value: this.params.octaves },
                uMode: { value: this.params.mode },
                uGrassColor: { value: this.biome.grass.clone() },
                uRockColor: { value: this.biome.rock.clone() }
            },
            vertexShader: `
                varying vec3 vPos;
                varying float vH;
                ${this.noiseSource}
                void main() {
                    vPos = (modelMatrix * vec4(position, 1.0)).xyz;
                    float h = fbm(vPos.xz * uFrequency) * uAmplitude;
                    vH = h;
                    vec3 p = position;
                    p.y = h;
                    gl_Position = projectionMatrix * modelViewMatrix * vec4(p, 1.0);
                }
            `,
            fragmentShader: `
                varying vec3 vPos;
                varying float vH;
                uniform vec3 uSunPos;
                uniform vec3 uFogColor;
                uniform float uFogDensity;
                uniform vec3 uGrassColor;
                uniform vec3 uRockColor;
                ${this.noiseSource}
                void main() {
                    float e = 0.5;
                    float hL = fbm((vPos.xz + vec2(-e, 0.0)) * uFrequency) * uAmplitude;
                    float hR = fbm((vPos.xz + vec2(e, 0.0)) * uFrequency) * uAmplitude;
                    float hD = fbm((vPos.xz + vec2(0.0, -e)) * uFrequency) * uAmplitude;
                    float hU = fbm((vPos.xz + vec2(0.0, e)) * uFrequency) * uAmplitude;
                    vec3 normal = normalize(vec3(hL - hR, 2.0 * e, hD - hU));
                    vec3 sunDir = normalize(uSunPos);
                    float diff = max(dot(normal, sunDir), 0.0);
                    vec3 color = mix(uGrassColor, uRockColor, clamp((1.0 - normal.y) * 1.5, 0.0, 1.0));
                    if (vH > 100.0) color = mix(color, vec3(1.0), clamp((vH - 100.0) / 30.0, 0.0, 1.0));
                    vec3 lighting = color * (0.2 + diff * 1.5);
                    float dist = length(cameraPosition - vPos);
                    float fog = 1.0 - exp(-uFogDensity * dist);
                    gl_FragColor = vec4(mix(lighting, uFogColor, clamp(fog, 0.0, 1.0)), 1.0);
                }
            `
        });

        this.terrain = new THREE.Mesh(terrainGeo, this.terrainMat);
        this.terrain.receiveShadow = true;
        this.scene.add(this.terrain);

        this.water = new THREE.Mesh(
            new THREE.PlaneGeometry(this.worldSize, this.worldSize),
            new THREE.MeshStandardMaterial({
                color: this.biome.water,
                transparent: true,
                opacity: this.params.waterOpacity,
                roughness: 0.15,
                metalness: 0.05
            })
        );
        this.water.rotateX(-Math.PI / 2);
        this.scene.add(this.water);
        this.updateWaterVisuals();
        this.updateSun();
    }

    populateControls() {
        this.populateSelect('input-biome-preset', this.library.biomes, this.params.activeBiome);
        this.populateSelect('input-tree-variant', this.library.treeVariants, this.params.selectedTreeVariant);
        this.populateSelect('input-rock-variant', this.library.rockVariants, this.params.selectedRockVariant);
        this.populateSelect('input-fern-variant', this.library.fernVariants, this.params.selectedFernVariant);
        this.populateSelect('input-material-preset', this.library.materialPresets, this.params.selectedMaterialPreset);
        this.syncControls();
        this.updateAssetNotes();
    }

    populateSelect(id, items, selectedId) {
        const select = document.getElementById(id);
        if (!select) return;
        select.innerHTML = '';
        items.forEach((item) => {
            const option = document.createElement('option');
            option.value = item.id;
            option.textContent = item.label;
            option.selected = item.id === selectedId;
            select.appendChild(option);
        });
    }

    syncControls() {
        const fieldMap = {
            'input-time': this.params.time,
            'input-fog': this.params.fog,
            'input-amplitude': this.params.amplitude,
            'input-frequency': this.params.frequency,
            'input-octaves': this.params.octaves,
            'input-exposure': this.params.exposure,
            'input-bloom': this.params.bloom,
            'input-water-level': this.params.waterLevel,
            'input-wave-strength': this.params.waveStrength,
            'input-wave-speed': this.params.waveSpeed,
            'input-tree-density': this.params.treeDensity,
            'input-rock-density': this.params.rockDensity,
            'input-fern-density': this.params.fernDensity,
            'input-distribution-min-height': this.params.distributionMinHeight,
            'input-distribution-max-height': this.params.distributionMaxHeight,
            'input-distribution-slope': this.params.distributionSlopeLimit,
            'input-scale-min': this.params.vegetationScaleMin,
            'input-scale-max': this.params.vegetationScaleMax,
            'input-grass-color': this.params.grassColor,
            'input-rock-color': this.params.rockColor
        };

        Object.entries(fieldMap).forEach(([id, value]) => {
            const el = document.getElementById(id);
            if (el) el.value = value;
        });

        const toggleMap = {
            'input-wireframe': this.params.wireframe,
            'input-autocycle': this.params.autoCycle
        };
        Object.entries(toggleMap).forEach(([id, value]) => {
            const el = document.getElementById(id);
            if (el) el.checked = value;
        });

        const selectMap = {
            'input-biome-preset': this.params.activeBiome,
            'input-tree-variant': this.params.selectedTreeVariant,
            'input-rock-variant': this.params.selectedRockVariant,
            'input-fern-variant': this.params.selectedFernVariant,
            'input-material-preset': this.params.selectedMaterialPreset
        };
        Object.entries(selectMap).forEach(([id, value]) => {
            const el = document.getElementById(id);
            if (el) el.value = value;
        });
    }

    getLibraryItem(collectionName, id) {
        return (this.library[collectionName] || []).find((item) => item.id === id) || null;
    }

    applyBiomePreset(id) {
        const biome = this.getLibraryItem('biomes', id);
        if (!biome) return;

        this.params.activeBiome = biome.id;
        this.params.selectedMaterialPreset = biome.materialPreset;
        this.params.selectedTreeVariant = biome.treeVariant;
        this.params.selectedRockVariant = biome.rockVariant;
        this.params.selectedFernVariant = biome.fernVariant;

        this.params.amplitude = biome.terrain.amplitude;
        this.params.frequency = biome.terrain.frequency;
        this.params.octaves = biome.terrain.octaves;

        this.params.treeDensity = biome.ecosystem.treeDensity;
        this.params.rockDensity = biome.ecosystem.rockDensity;
        this.params.fernDensity = biome.ecosystem.fernDensity;
        this.params.distributionMinHeight = biome.ecosystem.distributionMinHeight;
        this.params.distributionMaxHeight = biome.ecosystem.distributionMaxHeight;
        this.params.distributionSlopeLimit = biome.ecosystem.distributionSlopeLimit;
        this.params.vegetationScaleMin = biome.ecosystem.vegetationScaleMin;
        this.params.vegetationScaleMax = biome.ecosystem.vegetationScaleMax;

        this.params.waterLevel = biome.water.level;
        this.params.waveStrength = biome.water.waveStrength;
        this.params.waveSpeed = biome.water.waveSpeed;

        this.applyMaterialPreset(this.params.selectedMaterialPreset);
        this.updateTerrainUniforms();
        this.updateWaterVisuals();
        this.syncControls();
        this.updateAssetNotes();
        if (this.terrain) this.setupEcosystem();
    }

    applyMaterialPreset(id) {
        const material = this.getLibraryItem('materialPresets', id);
        if (!material) return;

        this.params.selectedMaterialPreset = material.id;
        this.params.grassColor = material.grassColor;
        this.params.rockColor = material.rockColor;
        this.params.waterColor = material.waterColor;
        this.params.waterOpacity = material.waterOpacity;

        this.biome.grass.set(material.grassColor);
        this.biome.rock.set(material.rockColor);
        this.biome.water.set(material.waterColor);
        this.terrainMat.uniforms.uGrassColor.value.copy(this.biome.grass);
        this.terrainMat.uniforms.uRockColor.value.copy(this.biome.rock);
        this.updateWaterVisuals();
        this.syncControls();
        this.updateAssetNotes();
    }

    updateTerrainUniforms() {
        this.terrainMat.uniforms.uAmplitude.value = this.params.amplitude;
        this.terrainMat.uniforms.uFrequency.value = this.params.frequency;
        this.terrainMat.uniforms.uOctaves.value = this.params.octaves;
        this.terrainMat.uniforms.uSeed.value = this.params.seed;
        this.terrainMat.uniforms.uFogDensity.value = this.params.fog;
    }

    updateWaterVisuals() {
        if (!this.water) return;
        this.water.material.color.set(this.params.waterColor);
        this.water.material.opacity = this.params.waterOpacity;
        this.water.position.y = this.params.waterLevel;
    }

    updateSun() {
        const angle = (this.params.time / 24) * Math.PI * 2 - Math.PI / 2;
        const pos = new THREE.Vector3(Math.cos(angle) * 12000, Math.sin(angle) * 12000, 2000);
        this.sun.position.copy(pos);
        this.sunMesh.position.copy(pos);
        this.terrainMat.uniforms.uSunPos.value.copy(pos);

        const daylight = Math.max(0, pos.y / 12000);
        const presetFog = this.getLibraryItem('materialPresets', this.params.selectedMaterialPreset)?.fogColor || '#95cfff';
        this.biome.fog = new THREE.Color().lerpColors(
            new THREE.Color(0x0b1220),
            new THREE.Color(presetFog),
            Math.min(1, daylight + 0.15)
        );
        this.terrainMat.uniforms.uFogColor.value.copy(this.biome.fog);
        this.terrainMat.uniforms.uFogDensity.value = this.params.fog;
        this.renderer.setClearColor(this.biome.fog.clone().multiplyScalar(daylight + 0.15));
        this.updateTelemetry();
    }

    updateTelemetry(dt) {
        const totalInstances = this.params.treeDensity + this.params.rockDensity + this.params.fernDensity;
        if (this.telemetry.instances) this.telemetry.instances.innerText = totalInstances.toLocaleString();
        if (this.telemetry.time) {
            const totalMinutes = Math.floor(this.params.time * 60) % (24 * 60);
            const hours = String(Math.floor(totalMinutes / 60)).padStart(2, '0');
            const minutes = String(totalMinutes % 60).padStart(2, '0');
            this.telemetry.time.innerText = `${hours}:${minutes}`;
        }
        if (dt && this.telemetry.fps) {
            this.telemetry.fps.innerText = Number.isFinite(dt) && dt > 0 ? String(Math.round(1 / dt)) : '--';
        }
    }

    setTime(value) {
        this.params.time = THREE.MathUtils.clamp(value, 0, 24);
        this.syncControls();
        this.updateSun();
    }

    updateEnvironment(dt) {
        if (this.params.autoCycle) {
            this.params.time = (this.params.time + dt * 0.75) % 24;
            this.syncControls();
            this.updateSun();
        }
        this.water.position.y = this.params.waterLevel + Math.sin(this.clock.elapsedTime * this.params.waveSpeed) * this.params.waveStrength;
    }

    clearEcosystem() {
        Object.values(this.ecosystem).forEach((entry) => {
            if (!entry) return;
            entry.parts.forEach((mesh) => {
                this.scene.remove(mesh);
                mesh.geometry.dispose();
                mesh.material.dispose();
            });
        });
        this.ecosystem = {};
    }

    setupEcosystem() {
        this.clearEcosystem();
        this.ecosystem.trees = this.createCategoryState('tree', this.params.selectedTreeVariant, this.params.treeDensity);
        this.ecosystem.rocks = this.createCategoryState('rock', this.params.selectedRockVariant, this.params.rockDensity);
        this.ecosystem.ferns = this.createCategoryState('fern', this.params.selectedFernVariant, this.params.fernDensity);
        this.spawnEcosystem();
        this.updateTelemetry();
    }

    createCategoryState(category, variantId, count) {
        const specs = this.getVariantSpecs(category, variantId);
        const parts = specs.map((spec) => {
            const mesh = new THREE.InstancedMesh(spec.geometry, spec.material, Math.max(1, count));
            mesh.castShadow = true;
            mesh.receiveShadow = category === 'rock';
            this.scene.add(mesh);
            return mesh;
        });
        return { category, variantId, parts };
    }

    spawnEcosystem() {
        this.spawnCategory(this.ecosystem.trees, this.params.treeDensity, {
            slopeMultiplier: 0.8,
            minOffsetFromWater: 6,
            scaleMultiplier: 2.1
        });
        this.spawnCategory(this.ecosystem.rocks, this.params.rockDensity, {
            slopeMultiplier: 1.25,
            minOffsetFromWater: 0,
            scaleMultiplier: 1.4
        });
        this.spawnCategory(this.ecosystem.ferns, this.params.fernDensity, {
            slopeMultiplier: 0.6,
            minOffsetFromWater: 2,
            scaleMultiplier: 1.2
        });
    }

    spawnCategory(state, count, options) {
        if (!state) return;
        const raycaster = new THREE.Raycaster();
        const up = new THREE.Vector3(0, 1, 0);
        const dummy = new THREE.Object3D();
        let placed = 0;
        let attempts = 0;
        const maxAttempts = Math.max(count * 7, 500);

        while (placed < count && attempts < maxAttempts) {
            attempts += 1;
            const x = (Math.random() - 0.5) * (this.worldSize * 0.82);
            const z = (Math.random() - 0.5) * (this.worldSize * 0.82);
            raycaster.set(new THREE.Vector3(x, 2000, z), new THREE.Vector3(0, -1, 0));
            const hit = raycaster.intersectObject(this.terrain);
            if (!hit.length) continue;

            const point = hit[0].point;
            const normal = hit[0].face.normal.clone().transformDirection(this.terrain.matrixWorld).normalize();
            const slope = 1 - Math.max(0, normal.dot(up));
            const maxSlope = Math.min(0.98, this.params.distributionSlopeLimit * options.slopeMultiplier);

            if (point.y < this.params.distributionMinHeight) continue;
            if (point.y > this.params.distributionMaxHeight) continue;
            if (point.y < this.params.waterLevel + options.minOffsetFromWater) continue;
            if (slope > maxSlope) continue;

            dummy.position.copy(point);
            dummy.quaternion.setFromUnitVectors(up, normal);
            dummy.rotateY(Math.random() * Math.PI * 2);

            const scale = THREE.MathUtils.lerp(
                this.params.vegetationScaleMin,
                this.params.vegetationScaleMax,
                Math.random()
            ) * options.scaleMultiplier;
            dummy.scale.setScalar(scale);
            dummy.updateMatrix();

            state.parts.forEach((mesh) => {
                mesh.setMatrixAt(placed, dummy.matrix);
            });
            placed += 1;
        }

        state.parts.forEach((mesh) => {
            mesh.count = placed;
            mesh.instanceMatrix.needsUpdate = true;
            mesh.computeBoundingSphere();
        });
    }

    getVariantSpecs(category, variantId) {
        const brown = new THREE.MeshLambertMaterial({ color: 0x6b4326 });
        const leaf = new THREE.MeshLambertMaterial({ color: this.biome.grass.clone().multiplyScalar(1.05) });
        const leafDark = new THREE.MeshLambertMaterial({ color: this.biome.grass.clone().multiplyScalar(0.8) });
        const rock = new THREE.MeshLambertMaterial({ color: this.biome.rock.clone() });
        const rockDark = new THREE.MeshLambertMaterial({ color: this.biome.rock.clone().multiplyScalar(0.82) });

        const branchGeo = (length, radiusTop, radiusBottom, y, zRot = 0, xRot = 0) => {
            const geometry = new THREE.CylinderGeometry(radiusTop, radiusBottom, length, 5);
            geometry.translate(0, y, 0);
            if (zRot) geometry.rotateZ(zRot);
            if (xRot) geometry.rotateX(xRot);
            return geometry;
        };

        if (category === 'tree' && variantId === 'pine') {
            return [
                { geometry: branchGeo(7, 0.45, 0.7, 3.5), material: brown.clone() },
                { geometry: new THREE.ConeGeometry(3.8, 12, 7).translate(0, 10, 0), material: leafDark.clone() },
                { geometry: new THREE.ConeGeometry(5.2, 14, 7).translate(0, 15, 0), material: leaf.clone() }
            ];
        }

        if (category === 'tree' && variantId === 'oak') {
            return [
                { geometry: branchGeo(8, 0.5, 0.8, 4), material: brown.clone() },
                { geometry: new THREE.DodecahedronGeometry(4.8, 0).translate(-1.8, 12, 0), material: leaf.clone() },
                { geometry: new THREE.DodecahedronGeometry(5.4, 0).translate(2.0, 13.5, 1.2), material: leafDark.clone() },
                { geometry: new THREE.DodecahedronGeometry(4.5, 0).translate(0, 16.2, -1.5), material: leaf.clone() }
            ];
        }

        if (category === 'tree' && variantId === 'dead') {
            return [
                { geometry: branchGeo(8, 0.35, 0.7, 4), material: brown.clone() },
                { geometry: branchGeo(4.2, 0.18, 0.28, 8, 0.95), material: brown.clone() },
                { geometry: branchGeo(3.8, 0.16, 0.24, 10, -0.9), material: brown.clone() },
                { geometry: branchGeo(2.8, 0.14, 0.2, 11.5, 0.45, 0.5), material: brown.clone() }
            ];
        }

        if (category === 'rock' && variantId === 'pebble') {
            return [
                { geometry: new THREE.DodecahedronGeometry(2.6, 0).translate(0, 1.6, 0), material: rock.clone() }
            ];
        }

        if (category === 'rock' && variantId === 'mossy') {
            return [
                { geometry: new THREE.IcosahedronGeometry(3.2, 0).translate(0, 2, 0), material: rockDark.clone() },
                { geometry: new THREE.BoxGeometry(3.4, 1.2, 2.4).translate(0.8, 1.2, 0), material: leafDark.clone() }
            ];
        }

        if (category === 'rock' && variantId === 'cliff') {
            return [
                { geometry: new THREE.OctahedronGeometry(4.1, 0).translate(0, 3.5, 0), material: rock.clone() },
                { geometry: new THREE.CylinderGeometry(1.1, 1.9, 6.2, 5).translate(-0.6, 3.1, 0.5), material: rockDark.clone() }
            ];
        }

        if (category === 'fern' && variantId === 'fan') {
            return [
                { geometry: new THREE.BoxGeometry(0.18, 3.8, 1.1).translate(0, 1.9, 0).rotateZ(0.55), material: leaf.clone() },
                { geometry: new THREE.BoxGeometry(0.18, 4.2, 1.0).translate(0, 2.1, 0).rotateZ(-0.1), material: leafDark.clone() },
                { geometry: new THREE.BoxGeometry(0.18, 3.8, 1.1).translate(0, 1.9, 0).rotateZ(-0.55), material: leaf.clone() }
            ];
        }

        if (category === 'fern' && variantId === 'bush') {
            return [
                { geometry: new THREE.ConeGeometry(0.8, 2.6, 5).translate(-0.5, 1.3, 0), material: leafDark.clone() },
                { geometry: new THREE.ConeGeometry(0.9, 3.0, 5).translate(0.45, 1.5, 0.3), material: leaf.clone() },
                { geometry: new THREE.ConeGeometry(0.7, 2.4, 5).translate(0.1, 1.1, -0.4), material: leaf.clone() }
            ];
        }

        return [
            { geometry: new THREE.BoxGeometry(0.18, 2.4, 0.8).translate(0, 1.2, 0).rotateZ(0.25), material: leaf.clone() },
            { geometry: new THREE.BoxGeometry(0.18, 2.7, 0.8).translate(0, 1.35, 0).rotateZ(-0.22), material: leafDark.clone() },
            { geometry: new THREE.BoxGeometry(0.18, 2.2, 0.8).translate(0, 1.1, 0).rotateX(0.4), material: leaf.clone() }
        ];
    }

    updateAssetNotes() {
        this.updateNote('input-biome-preset', 'biomes', 'biome-note');
        this.updateNote('input-tree-variant', 'treeVariants', 'tree-note');
        this.updateNote('input-rock-variant', 'rockVariants', 'rock-note');
        this.updateNote('input-fern-variant', 'fernVariants', 'fern-note');
        this.updateNote('input-material-preset', 'materialPresets', 'material-note');
    }

    updateNote(controlId, collectionName, noteId) {
        const note = document.getElementById(noteId);
        const control = document.getElementById(controlId);
        if (!note || !control) return;
        const item = this.getLibraryItem(collectionName, control.value);
        if (!item) {
            note.textContent = '';
            return;
        }
        const sourceLink = item.source ? `<a href="${item.source}" target="_blank" rel="noreferrer">source</a>` : '';
        const license = item.license ? ` · ${item.license}` : '';
        note.innerHTML = `${item.description}${sourceLink ? ` · ${sourceLink}` : ''}${license}`;
    }

    animate() {
        requestAnimationFrame(() => this.animate());
        const dt = this.clock.getDelta();
        this.updateEnvironment(dt);

        if (document.pointerLockElement === this.canvas) {
            const moveDir = new THREE.Vector3();
            if (this.cam.keys.KeyW) moveDir.add(this.cam.front);
            if (this.cam.keys.KeyS) moveDir.sub(this.cam.front);
            if (this.cam.keys.KeyA) moveDir.sub(this.cam.right);
            if (this.cam.keys.KeyD) moveDir.add(this.cam.right);
            if (moveDir.lengthSq() > 0) moveDir.normalize();
            this.cam.velocity.lerp(moveDir.multiplyScalar(this.cam.speed), this.cam.damping);
            this.cam.pos.add(this.cam.velocity);
            this.camera.position.copy(this.cam.pos);
            this.camera.lookAt(this.cam.pos.clone().add(this.cam.front));
        }

        this.terrainMat.wireframe = this.params.wireframe;
        this.renderer.toneMappingExposure = this.params.exposure;
        this.renderer.render(this.scene, this.camera);
        this.updateTelemetry(dt);
    }

    onResize() {
        this.camera.aspect = window.innerWidth / window.innerHeight;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(window.innerWidth, window.innerHeight);
        this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    }

    setupEvents() {
        window.addEventListener('keydown', (e) => {
            this.cam.keys[e.code] = true;
            if (e.code === 'Escape' && document.pointerLockElement === this.canvas) {
                document.exitPointerLock();
            }
        });
        window.addEventListener('keyup', (e) => { this.cam.keys[e.code] = false; });
        this.canvas.addEventListener('mousedown', () => this.canvas.requestPointerLock());
        window.addEventListener('resize', () => this.onResize());
        document.addEventListener('pointerlockchange', () => {
            this.uiRoot.classList.toggle('immersive', document.pointerLockElement === this.canvas);
        });
        window.addEventListener('mousemove', (e) => {
            if (document.pointerLockElement !== this.canvas) return;
            this.cam.yaw += e.movementX * this.cam.sensitivity;
            this.cam.pitch -= e.movementY * this.cam.sensitivity;
            const ry = THREE.MathUtils.degToRad(this.cam.yaw);
            const rp = THREE.MathUtils.degToRad(Math.max(-89, Math.min(89, this.cam.pitch)));
            this.cam.front.set(
                Math.cos(ry) * Math.cos(rp),
                Math.sin(rp),
                Math.sin(ry) * Math.cos(rp)
            ).normalize();
            this.cam.right.crossVectors(this.cam.front, this.cam.worldUp).normalize();
        });

        const bindRange = (id, param, onChange) => {
            const el = document.getElementById(id);
            if (!el) return;
            el.oninput = (e) => {
                this.params[param] = parseFloat(e.target.value);
                if (onChange) onChange(this.params[param]);
            };
        };
        const bindToggle = (id, param, onChange) => {
            const el = document.getElementById(id);
            if (!el) return;
            el.onchange = (e) => {
                this.params[param] = e.target.checked;
                if (onChange) onChange(this.params[param]);
            };
        };
        const bindSelect = (id, param, onChange) => {
            const el = document.getElementById(id);
            if (!el) return;
            el.onchange = (e) => {
                this.params[param] = e.target.value;
                if (onChange) onChange(this.params[param]);
                this.updateAssetNotes();
            };
        };
        const bindColor = (id, param, onChange) => {
            const el = document.getElementById(id);
            if (!el) return;
            el.oninput = (e) => {
                this.params[param] = e.target.value;
                if (onChange) onChange(this.params[param]);
            };
        };

        bindRange('input-amplitude', 'amplitude', () => { this.updateTerrainUniforms(); this.setupEcosystem(); });
        bindRange('input-frequency', 'frequency', () => { this.updateTerrainUniforms(); this.setupEcosystem(); });
        bindRange('input-octaves', 'octaves', () => { this.updateTerrainUniforms(); this.setupEcosystem(); });
        bindRange('input-time', 'time', () => this.updateSun());
        bindRange('input-fog', 'fog', () => this.updateSun());
        bindRange('input-water-level', 'waterLevel', () => this.updateWaterVisuals());
        bindRange('input-wave-strength', 'waveStrength');
        bindRange('input-wave-speed', 'waveSpeed');
        bindRange('input-tree-density', 'treeDensity', () => this.setupEcosystem());
        bindRange('input-rock-density', 'rockDensity', () => this.setupEcosystem());
        bindRange('input-fern-density', 'fernDensity', () => this.setupEcosystem());
        bindRange('input-distribution-min-height', 'distributionMinHeight', () => this.setupEcosystem());
        bindRange('input-distribution-max-height', 'distributionMaxHeight', () => this.setupEcosystem());
        bindRange('input-distribution-slope', 'distributionSlopeLimit', () => this.setupEcosystem());
        bindRange('input-scale-min', 'vegetationScaleMin', () => this.setupEcosystem());
        bindRange('input-scale-max', 'vegetationScaleMax', () => this.setupEcosystem());
        bindRange('input-exposure', 'exposure');
        bindRange('input-bloom', 'bloom');
        bindColor('input-grass-color', 'grassColor', () => {
            this.biome.grass.set(this.params.grassColor);
            this.terrainMat.uniforms.uGrassColor.value.copy(this.biome.grass);
            this.setupEcosystem();
        });
        bindColor('input-rock-color', 'rockColor', () => {
            this.biome.rock.set(this.params.rockColor);
            this.terrainMat.uniforms.uRockColor.value.copy(this.biome.rock);
            this.setupEcosystem();
        });
        bindToggle('input-wireframe', 'wireframe');
        bindToggle('input-autocycle', 'autoCycle');

        bindSelect('input-biome-preset', 'activeBiome', (value) => this.applyBiomePreset(value));
        bindSelect('input-tree-variant', 'selectedTreeVariant', () => this.setupEcosystem());
        bindSelect('input-rock-variant', 'selectedRockVariant', () => this.setupEcosystem());
        bindSelect('input-fern-variant', 'selectedFernVariant', () => this.setupEcosystem());
        bindSelect('input-material-preset', 'selectedMaterialPreset', (value) => this.applyMaterialPreset(value));

        document.getElementById('btn-play').onclick = () => this.canvas.requestPointerLock();
        document.getElementById('btn-regenerate').onclick = () => {
            this.params.seed = Math.random() * 1000;
            this.updateTerrainUniforms();
            this.setupEcosystem();
        };

        document.querySelectorAll('.tab-btn').forEach((btn) => {
            btn.onclick = () => {
                document.querySelectorAll('.tab-btn').forEach((item) => item.classList.remove('active'));
                document.querySelectorAll('.tab-content').forEach((content) => content.classList.add('hidden'));
                btn.classList.add('active');
                const target = document.getElementById(btn.dataset.tab);
                if (target) target.classList.remove('hidden');
            };
        });

        window.engine = this;
        this.updateTelemetry();
    }
}

window.onload = () => { new PETSEngine(); };
