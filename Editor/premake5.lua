
project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/bin")

    files {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs {
        "src",
        "src/**",

        "src/Magma",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",
        "%{RootPath}/VolcaniCore/src/impl",

        "%{RootPath}/Magma/src",
        "%{RootPath}/Magma/src/Magma",

        "%{RootPath}/Lava/src",
        "%{RootPath}/Lava/src/**",

        "%{Includes.glm}",
        "%{Includes.glad}",
        "%{Includes.glfw}",

        "%{VendorPaths.glslang}",
        "%{Includes.glslang}",
        "%{Includes.SPIRV_Cross}",
        "%{Includes.efsw}",
        "%{Includes.miniz_cpp}",

        "%{Includes.assimp}",
        "%{Includes.stb_image}",
        "%{Includes.freetype}",

        "%{Includes.imgui}",
        "%{Includes.imgui}/imgui",
        "%{Includes.ImGuiFileDialog}",
        "%{Includes.IconFontCppHeaders}",

        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",

        "%{Includes.clay}",
        "%{Includes.flecs}",
        "%{Includes.PhysX}",

        "%{Includes.angelscript}",
        "%{MagmaVendorDir}",
        "%{Includes.soloud}",
        "%{Includes.asio}",
        "%{Includes.lmdb}",
    }

    links {
        "Lava",
        "Magma",
        "VolcaniCore",

        "glfw",
        "glad",

        "glslang",
        "SPIRV-Cross",

        "assimp",
        "freetype",
        "stb_image",
        "efsw",

        "imgui",
        "ImGuiFileDialog",
        "ImGuizmo",
        "ImGuiColorTextEdit",

        "flecs",
        "yaml-cpp",
        "angelscript",
        "soloud",
        "asio",
        "lmdb",

        "ssl",
        "crypto"
    }

    defines {
        "flecs_STATIC",
        "NOMINMAX",
        "YAML_CPP_STATIC_DEFINE"
    }

    filter "toolset:msc or system:linux"
        links "PhysX"
        debugdir ".."

    filter "system:linux"
        links {
            "pthread",
            "dl",
            "GL",
            "X11",
        }

    filter "system:windows"
        systemversion "latest"
        links {
            "gdi32",
            "kernel32",
            "psapi",
            "Ws2_32",
        }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }

include "Editor/.deps/imgui"
include "Editor/.deps/ImGuiFileDialog"
include "Editor/.deps/ImGuizmo"
include "Editor/.deps/ImGuiColorTextEdit"
include "Editor/.deps/assimp"
include "Editor/.deps/freetype"
include "Editor/.deps/stb_image"
include "Editor/.deps/glslang"
include "Editor/.deps/SPIRV-Cross"
include "Editor/.deps/efsw"
