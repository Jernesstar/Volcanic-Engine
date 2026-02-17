project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Editor/obj")
    targetdir ("%{RootPath}/build/Editor/bin")

    files {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs {
        "src",
        "src/**",
        "%{RootPath}",
        "%{EditorVendorDir}",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",
        "%{RootPath}/VolcanicWindow/**",

        "%{RootPath}/Editor/src",
        "%{RootPath}/Editor/src/**",

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
        "angelscript",

        "efsw",
        "soloud",
        "stb_image",
        "freetype",
        "yaml-cpp",
        "rapidjson",

        "glslang",
        "SPIRV-Cross",

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

include "Editor/.deps/stb_image"
include "Editor/.deps/efsw"
include "Editor/.deps/yaml-cpp"
include "Editor/.deps/freetype"
include "Editor/.deps/glslang"
include "Editor/.deps/SPIRV-Cross"
