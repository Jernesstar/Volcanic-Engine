project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    files {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs {
        "src",
        "src/**",
        "%{RootPath}",
        "%{EngineVendorDir}",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",

        "%{RootPath}/Engine/src",
        "%{RootPath}/Engine/src/**",

        "%{Includes.glm}",
        "%{Includes.glfw}",
        "%{Includes.spdlog}",

        "%{Includes.angelscript}",
        "%{Includes.flecs}",
        "%{Includes.glad}",
        "%{Includes.lmdb}",
        "%{Includes.soloud}",
        "%{VendorPaths.angelscript}",
    }

    links {
        "VolcaniCore",

        "glfw",

        "angelscript",
        "flecs",
        "glad",
        "lmdb",
        "soloud",
        "uSockets",

        "z",
        "ssl",
        "crypto",
        "uv"
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
        }

    filter "system:windows"
        systemversion "latest"
        links {
            "dbghelp",
            "gdi32",
            "kernel32",
            "psapi",
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

include "Engine/.deps/angelscript"
include "Engine/.deps/glad"
include "Engine/.deps/soloud"
include "Engine/.deps/lmdb"
include "Engine/.deps/flecs"
include "Engine/.deps/uWebSockets"
-- include "Engine/.deps/Jolt"
