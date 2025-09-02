project "Magma"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/lib")

    files {
        "src/Magma/**.h",
        "src/Magma/**.cpp"
    }

    includedirs {
        "src/Magma",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",
        "%{RootPath}/VolcaniCore/src/impl",

        "%{RootPath}/Magma/src",
        "%{RootPath}/Magma/src/Magma",

        "%{Includes.glm}",
        "%{Includes.glad}",
        "%{Includes.glfw}",

        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",

        "%{Includes.clay}",
        "%{Includes.flecs}",
        "%{Includes.PhysX}",

        "%{Includes.angelscript}",
        "%{MagmaVendorDir}",
        "%{Includes.soloud}"
    }

    links {
        "VolcaniCore",

        "yaml-cpp",

        "flecs",
        "angelscript",
        "soloud"
    }

    defines {
        "flecs_STATIC",
        "NOMINMAX",
        "YAML_CPP_STATIC_DEFINE"
    }

    filter "action:vs* or system:linux"
        links { "PhysX" }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }

    filter "system:windows"
        systemversion "latest"


include "Magma/.deps/angelscript"
include "Magma/.deps/soloud"
include "Magma/.deps/yaml-cpp"
include "Magma/.deps/glad"


project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/bin")

    files {
        "src/Editor/**.h",
        "src/Editor/**.cpp",
    }

    includedirs {
        "src/Editor",
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
        "%{VendorPaths.angelscript}",
        "%{Includes.soloud}",
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
        "soloud"
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

include "Magma/.deps/imgui"
include "Magma/.deps/ImGuiFileDialog"
include "Magma/.deps/ImGuizmo"
include "Magma/.deps/ImGuiColorTextEdit"
include "Magma/.deps/assimp"
include "Magma/.deps/freetype"
include "Magma/.deps/stb_image"
include "Magma/.deps/efsw"
include "Magma/.deps/glslang"
include "Magma/.deps/SPIRV-Cross"
