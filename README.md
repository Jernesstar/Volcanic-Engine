# VolcanicEngine

VolcanicEngine is a modular, high-performance C++20 game engine built around a scriptable-first philosophy: game developers write exclusively in **AngelScript** — no C++ required at the game layer. The engine handles rendering, physics, audio, ECS, and UI through a clean layered architecture, exposing all of it to scripts through a first-class binding surface.

---

## 🏗️ Architecture

The engine is divided into four primary modules:

| Module | Role |
|---|---|
| **VolcaniCore** | Cross-platform foundation: windowing (GLFW), input, logging (spdlog), and core utility types (`Buffer`, `List`, `UUID`) |
| **Engine** | Core runtime: ECS (flecs), deferred rendering (OpenGL 4.6), Jolt Physics, AngelScript integration, SoLoud audio |
| **Editor** | Desktop authoring tool: scene editing, asset management, live debugging, built on ImGui |
| **Runtime** | Streamlined execution environment for shipping built Volcanic projects |

---

## 🌍 Triple-World ECS Architecture

Every `Scene` in VolcanicEngine contains three independent ECS worlds, each backed by **flecs** and processed on separate rendering paths. This allows 3D gameplay, 2D sprites, and UI to coexist cleanly without architectural compromise.

### World3D

The primary 3D simulation world. Systems registered here drive:

- **Meshes & Materials**: `MeshComponent` with per-slot `MaterialInstance` overrides. Geometry (topology) and Material (surface description) are separate registered assets, decomposed at import time from embedded FBX/OBJ materials via Assimp.
- **Lighting**: Directional, Point, and Spotlights feeding a deferred G-Buffer shading pass with cascaded shadow maps.
- **Physics**: Jolt rigid body simulation and collision detection.
- **Particles**: Compute-shader-driven particle systems simulated and emitted entirely on the GPU.

### World2D

A dedicated world for 2D content — sprites, parallax layers, and 2D lighting — enabling mixed-dimension projects without polluting 3D world state. The 2D lighting model uses a **1D polar-coordinate shadow map** for point light occlusion and a normal-map accumulation pass for diffuse contribution.

### Canvas

A specialized ECS world for UI. Rather than a traditional scene-graph hierarchy, layout and rendering are driven by components:

- `RectComponent` — position, size, and anchor in screen space
- `LayoutComponent` — automatic vertical/horizontal alignment and spacing
- `ImageComponent` — textured quads
- `TextComponent` — font-atlas-based text rendering (font system integration in progress)
- `ButtonComponent` — ECS-driven input event propagation

Canvas animation uses a four-layer stack: **tweens → keyframe clips → state machines → sequencers**, with a computed style pattern (`CanvasStyleComponent → AnimatedStyleComponent → ComputedStyleComponent`) that prevents animation from clobbering base property values.

**Final composite order**: `World3D → World2D (with lighting) → tonemap → Canvas → screen`

---

## 🎨 Rendering Pipeline

### Default Render Pipeline

A deferred renderer with the following pass structure:

1. **Shadow Pass** — Cascaded shadow maps for directional lights
2. **G-Buffer Pass** — Position, normal, albedo/roughness/metallic written to MRT
3. **Lighting Pass** — Deferred shading from all light sources + shadow lookup
4. **Particle Pass** — Compute-simulated particles composited over the lit scene
5. **Post-Process** — Multi-pass compute Bloom (downsample/upsample chain) → Tonemapping
6. **World2D** — Lit 2D sprites composited over the tonemapped 3D frame
7. **Canvas** — UI composited last, always on top

### Scriptable Render Hooks

The `DefaultRenderPipeline` exposes **14 named injection points** via a `PipelineStage` enum (`PreGeometry`, `PostShadows`, `PrePostProcess`, etc.). Scripts register hook callbacks that receive a `ScriptPipelineContext` — a safe, read-only view of named framebuffers at that stage boundary.

```angelscript
void OnPrePostProcess(PipelineContext@ ctx)
{
    Framebuffer@ gbuffer = ctx.GetFramebuffer("GBuffer");
    // sample, blit, or redirect output
}
```

### Fully Custom Script Pipelines

For complete control, a script class can implement the `IRenderPipeline` interface. `RenderPipelineLoader` discovers the class by name in the project's script modules, instantiates it, and wraps it in a `ScriptRenderPipeline` C++ proxy that dispatches all virtual calls into the AngelScript object. The `SceneRenderer` holds a `Ref<RenderPipeline>` that can be switched at runtime between the default+hooks pipeline and a fully custom one.

### Custom Shader Materials

Materials are defined as a two-tier structure:

- **`Material`** — fixed PBR fields (albedo, normal, roughness, metallic, emissive) + an open `std::variant`-based property map (`MaterialProperty`) accepting `int`, `float`, `vec2`–`vec4`, `mat4`, or `Texture`
- **`MaterialInstance`** — a sparse override layer on top of a base `Material`; only differing properties are stored

A `MaterialBinder` resolves the full property set (instance overrides merged with base defaults) and writes into the existing `Uniforms` system before each draw call. SPIRV-Cross reflection is used to validate property names against the bound shader at bind time.

---

## 📦 Asset System

### Asset Identity & Pipeline

All assets are tracked by stable **UUIDs** so references survive file moves and renames. The `EditorAssetManager` drives a build pipeline that converts raw source files (FBX, PNG, GLSL, `.prefab`) into optimized binary blobs consumed by the Runtime.

Hot reloading via **efsw** (embedded file system watcher) triggers live updates to textures, shaders, and scripts without restarting the editor.

### Geometry vs. Material Separation

Import decomposes 3D models into independent registered assets:

- `Geometry` — vertex/index buffers, topology, LOD data
- `Material` — surface description, shader reference, property defaults

`MeshComponent` holds an array of `(Geometry, MaterialInstance)` slot pairs, allowing per-slot material overrides without duplicating geometry.

### Prefab System

Compound entities are authored as `.prefab` YAML assets. The `PrefabLibrary` registers them by name; instantiation spawns the full entity hierarchy into the target ECS world. `ComponentRegistry` provides string-dispatch serialization/deserialization so component types registered in C++ can be round-tripped through the YAML format without bespoke per-type code.

---

## ✍️ AngelScript Integration

AngelScript scriptability is a **first-class constraint** on every system design decision. C++ interfaces are designed with script exposure in mind from the start. Script-facing types include:

- `ScriptFramebuffer`, `ScriptRenderPass`, `ScriptRenderer` — render pipeline access
- `ScriptPipelineContext` — safe framebuffer read access inside hooks
- `ScriptRenderPipeline` — full pipeline override via AngelScript class
- ECS component types — read/write access to all engine components from script
- Asset handles — `AssetID`-based lookups for meshes, textures, materials, prefabs

---

## 🛠️ Getting Started

### Prerequisites

- **Compiler**: C++20 (GCC 11+, Clang 13+, MSVC 2022)
- **Build System**: Premake5
- **Graphics**: OpenGL 4.6+

### Build

```bash
./.scripts/premake.sh   # Generate project files
./.scripts/build.sh     # Build all modules
./.scripts/run.sh       # Launch the Editor
```

### Dependencies

| Library | Purpose |
|---|---|
| flecs | Entity Component System |
| Jolt Physics | Rigid body simulation |
| AngelScript | Scripting layer |
| SPIRV-Cross | Shader reflection & property validation |
| Assimp | 3D model import |
| ImGui | Editor UI |
| spdlog | Logging |
| efsw | Hot reload file watching |
| SoLoud | Audio |
| GLFW | Windowing & input |

---

## 📜 License

This project is licensed under the terms in [LICENSE.md](LICENSE.md).