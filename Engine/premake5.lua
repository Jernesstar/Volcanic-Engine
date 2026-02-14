project "Engine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/bin")

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
        "%{RootPath}/VolcanicWindow/**",

        "%{RootPath}/Engine/src",
        "%{RootPath}/Engine/src/**",

        "%{Includes.glm}",
        "%{Includes.glad}",
        "%{Includes.glfw}",
        "%{Includes.angelscript}",
        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",
        "%{Includes.efsw}",
        "%{Includes.stb_image}",
        "%{Includes.soloud}",
        "%{Includes.freetype}",
    }

    links {
        "VolcaniCore",
        "VolcanicWindow",

        "glfw",

        "glad",
        "efsw",
        "soloud",
        "stb_image",

        "freetype",
        "yaml-cpp",
        "angelscript",

        "z"
    }

    defines {
        "NOMINMAX",
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

include "Engine/.deps/angelscript"
include "Engine/.deps/glad"
include "Engine/.deps/soloud"
include "Engine/.deps/stb_image"
include "Engine/.deps/efsw"
include "Engine/.deps/yaml-cpp"
include "Engine/.deps/freetype"
