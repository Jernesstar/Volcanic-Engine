
project "Magma"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Magma/obj")
    targetdir ("%{RootPath}/build/Magma/bin")

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
        "%{RootPath}/VolcanicWindow/**",

        "%{RootPath}/Magma/src",
        "%{RootPath}/Magma/src/Magma",

        "%{RootPath}/Lava/src",
        "%{RootPath}/Lava/src/**",

        "%{Includes.glm}",
        "%{Includes.glad}",
        "%{Includes.glfw}",

        "%{Includes.angelscript}",
        "%{MagmaVendorDir}",
        "%{LavaVendorDir}",

        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",

        "%{Includes.efsw}",
        "%{Includes.libgit2}",
        "%{Includes.cpp_httplib}",

        "%{Includes.stb_image}",
        "%{Includes.soloud}",
        "%{Includes.miniz_cpp}",

        "%{Includes.clay}",
        "%{Includes.IconFontCppHeaders}",
    }

    links {
        "Lava",
        "VolcaniCore",
        "VolcanicWindow",

        "glfw",

        "glad",
        "efsw",
        "libgit2",
        "soloud",
        "stb_image",

        "yaml-cpp",
        "angelscript",

        "ssl",
        "crypto",

        "z"
    }

    defines {
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
        "YAML_CPP_STATIC_DEFINE",
        "_WIN32_WINNT=0x0A00",
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
        }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }


include "Magma/.deps/glad"
include "Magma/.deps/soloud"
include "Magma/.deps/stb_image"
include "Magma/.deps/efsw"
include "Magma/.deps/libgit2"
include "Magma/.deps/yaml-cpp"
