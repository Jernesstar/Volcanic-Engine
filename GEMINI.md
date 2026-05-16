# VolcanicEngine

VolcanicEngine is a modern, modular development platform and game engine designed for programmers of all levels of expertise. It features a layered architecture with a core foundation, a high-level engine, and specific entry points for Editor and Runtime environments.

## Architecture Overview

The project is organized into four main modules:

1.  **VolcaniCore:** The foundational layer providing essential utilities, application lifecycle management, windowing abstraction (via GLFW), and logging (via spdlog).
2.  **Engine:** The core engine logic, including:
    *   **Graphics:** A platform-agnostic rendering API with implementations for OpenGL (and hooks for others). Supports 2D and 3D rendering, materials, meshes, and shaders.
    *   **ECS:** Entity Component System powered by **flecs**.
    *   **Asset Management:** System for loading and managing engine assets.
    *   **Physics:** Integration with **Jolt** physics engine.
    *   **Scripting:** Support for **AngelScript**.
    *   **Audio:** Integration with **SoLoud**.
3.  **Editor:** A desktop application for content creation, utilizing **ImGui** for the user interface.
4.  **Runtime:** The execution environment for published applications.

## Technologies & Dependencies

*   **Language:** C++ (C++latest/C++20)
*   **Build System:** Premake5
*   **Windowing/Input:** GLFW
*   **Graphics API:** OpenGL (via glad), support for GLM
*   **ECS:** flecs
*   **Physics:** Jolt
*   **Scripting:** AngelScript
*   **UI:** ImGui, ImGuizmo
*   **Logging:** spdlog
*   **Utilities:** yaml-cpp, rapidjson, stb_image, assimp, freetype

## Building and Running

The project uses Premake5 to generate build files (Makefiles by default on Linux).

### Prerequisites
*   Premake5 installed and in your PATH.
*   C++ compiler with C++20 support (GCC/Clang).

### Commands

*   **Generate Build Files:**
    ```bash
    ./.scripts/premake.sh
    ```
*   **Build Project:**
    ```bash
    ./.scripts/build.sh [TargetName]
    ```
    *   `TargetName` defaults to `Makefile` (all projects).
    *   Example: `./.scripts/build.sh Editor`
*   **Run Project:**
    ```bash
    ./.scripts/run.sh
    ```
*   **Update/Install (Linux):**
    ```bash
    ./.scripts/update.sh
    ```
    *Note: This script copies the built binary to `/opt/Volcanic/EngineEditor`.*

## Development Conventions

*   **Header Guards:** Use `#pragma once`.
*   **Namespacing:** Most core code resides within the `VolcaniCore` or `Engine` namespaces.
*   **Smart Pointers:** The project uses a custom `Ref<T>` and `Scope<T>` (likely aliases for `std::shared_ptr` and `std::unique_ptr` found in `Defines.h`).
*   **Assertions:** Use `VOLCANIC_ASSERT` or similar macros defined in `Core/Assert.h`.
*   **Logging:** Use `VolcaniCore::Log` for formatted logging output.

## Directory Structure

*   `.scripts/`: Build and utility scripts.
*   `VolcaniCore/`: Foundational engine code.
*   `Engine/`: Core engine systems (Graphics, ECS, Physics, etc.).
*   `Editor/`: Editor application source and assets.
*   `Runtime/`: Runtime application source.
*   `build/`: Output directory for generated build files and binaries (created by Premake).
