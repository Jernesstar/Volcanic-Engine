# VolcanicEngine

VolcanicEngine is a modular, high-performance C++20 game engine and development platform. It is built with a focus on scriptable flexibility, a clean layered architecture, and modern rendering techniques.

## 🏗️ Architecture Overview

The engine is divided into four primary modules:

1.  **VolcaniCore**: The foundational layer providing cross-platform abstractions for windowing (GLFW), input, logging (spdlog), and core utility types (Buffer, List, UUID).
2.  **Engine**: The core systems including a deferred rendering pipeline, ECS (flecs), Jolt Physics, AngelScript integration, and SoLoud audio.
3.  **Editor**: A comprehensive desktop application for scene editing, asset management, and live debugging, built using ImGui.
4.  **Runtime**: A streamlined execution environment for running built Volcanic projects.

---

## 🌍 The 3-Way Scene Split

VolcanicEngine utilizes a unique triple-world ECS structure within every `Scene`. This allows for strict separation of concerns and optimized processing paths for different types of content:

### 1. World3D
Designed for traditional 3D gameplay. It handles:
*   **Geometry & Materials**: Static and dynamic meshes with property-based materials.
*   **Lighting**: Directional, Point, and Spotlights for deferred shading.
*   **Physics**: Rigid body simulation and collision detection.
*   **Particles**: High-performance compute-shader-driven particle systems.

### 2. World2D
A dedicated world for 2D sprites, parallax backgrounds, and 2D lighting, allowing for mixed-dimension projects without architectural friction.

### 3. Canvas
A specialized world for User Interfaces. Instead of a traditional hierarchy-only UI, the Canvas uses ECS for layout and rendering:
*   **Components**: `RectComponent`, `TextComponent`, `ImageComponent`, `ButtonComponent`.
*   **Layout Engine**: `LayoutComponent` for automatic vertical/horizontal alignment and spacing.
*   **Interactivity**: ECS-based event propagation for UI elements.

---

## 🎨 Scriptable Rendering Pipeline

The engine features a highly extensible rendering architecture centered around the `DefaultRenderPipeline` and its integration with **AngelScript**.

### Default Render Pipeline
A sophisticated deferred renderer featuring:
*   **Shadow Mapping**: Cascaded shadow maps for directional lights.
*   **G-Buffer Pass**: Efficient storage of position, normal, and albedo data.
*   **Compute Particles**: Simulation and emission handled entirely on the GPU.
*   **Post-Processing**: A multi-pass compute-based Bloom (Downsample/Upsample chain) and Tonemapping.

### Script Hooks & Pipelines
VolcanicEngine empowers developers to modify the rendering flow without touching C++:
*   **Pipeline Hooks**: Inject custom logic at 14 distinct stages, including `PreGeometry`, `PostShadows`, and `PrePostProcess`.
*   **Redirection**: Scripts can redirect the pipeline's output to custom `Framebuffers` for specialized effects (e.g., pixel-art downscaling).
*   **Custom Pipelines**: Developers can implement the entire `IRenderPipeline` interface in AngelScript to build completely custom rendering paths.

---

## 📦 Editor & Asset Management

The Volcanic Editor provides a professional workflow for managing game content.

### Asset Management System
*   **Binary Building**: The `EditorAssetManager` converts raw source files (FBX, PNG, GLSL) into optimized binary blobs for the engine.
*   **Hot Reloading**: Integrated file watching (`efsw`) enables real-time updates of textures, shaders, and scripts without restarting the editor.
*   **UUID Persistence**: Assets are tracked via stable UUIDs, ensuring robust referencing even when files are moved.

### Material System
Volcanic features a flexible property-based material system:
*   **Material Blueprints**: Define a shader and a set of default properties (Colors, Scalars, Textures).
*   **Material Instances**: Create variations of materials with specific property overrides, reducing draw call overhead and simplifying asset management.
*   **Assimp Integration**: Automatically extracts material properties and textures during 3D model import.

---

## 🛠️ Getting Started

### Prerequisites
*   **Compiler**: C++20 compatible (GCC 11+, Clang 13+, MSVC 2022).
*   **Build System**: Premake5.
*   **Graphics**: OpenGL 4.6+ support.

### Build Commands
```bash
./.scripts/premake.sh  # Generate project files
./.scripts/build.sh    # Build all modules
./.scripts/run.sh      # Launch the Editor
```

## 📜 License
This project is licensed under the [LICENSE.md](LICENSE.md).
