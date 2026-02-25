project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Editor/obj")
    targetdir ("%{RootPath}/build/Editor/bin")

    files {
        "src/Editor/App/**.cpp",
        -- "src/**.cpp",
    }

    includedirs {
        "src",
        "src/**",
        "%{RootPath}",
        "%{EngineVendorDir}",
        "%{EditorVendorDir}",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/**",
        "%{RootPath}/Engine/src",
        "%{RootPath}/Engine/src/**",

        "%{RootPath}/Editor/src",
        "%{RootPath}/Editor/src/**",

        "%{Includes.glm}",
        "%{Includes.glfw}",
        "%{Includes.spdlog}",

        "%{Includes.angelscript}",
        "%{Includes.flecs}",
        "%{Includes.glad}",
        "%{Includes.lmdb}",
        "%{Includes.soloud}",
        "%{VendorPaths.angelscript}",

        "%{Includes.assimp}",
        "%{Includes.glslang}",
        "%{Includes.SPIRVCross}",
        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",
        "%{Includes.efsw}",
        "%{Includes.stb_image}",
        "%{Includes.freetype}",
    }

    links {
        "VolcaniCore",
        "Engine",

        "glfw",

        "angelscript",
        "flecs",
        "glad",
        "lmdb",
        "soloud",

        "assimp",
        "efsw",
        "stb_image",
        "freetype",
        "yaml-cpp",
        "glslang",
        "SPIRV-Cross",

        "z"
    }

    defines {
        "NOMINMAX",
        "NDEBUG",
        "WIN32_LEAN_AND_MEAN",
        "YAML_CPP_STATIC_DEFINE",
    }

    filter "system:linux"
        links {
            "pthread",
            "dl",
            "GL",
            "X11",
            "wayland-client"
        }

    filter "system:windows"
        systemversion "latest"
        links {
            "gdi32",
            "kernel32",
            "psapi",
            "z",
            "Ws2_32",
            "advapi32",
            "rpcrt4",
            "crypt32",
            "stdc++exp"
        }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }

include "Editor/.deps/assimp"
include "Editor/.deps/stb_image"
include "Editor/.deps/efsw"
include "Editor/.deps/yaml-cpp"
include "Editor/.deps/freetype"
include "Editor/.deps/glslang"
include "Editor/.deps/SPIRV-Cross"
