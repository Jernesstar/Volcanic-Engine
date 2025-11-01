
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

        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",

        "%{Includes.efsw}",
        "%{Includes.libgit2}",

        "%{Includes.drogon}",
        "%{VendorPaths.drogon}/*",
        "%{VendorPaths.drogon}/orm_lib/inc",
        "%{VendorPaths.drogon}/nosql_lib/redis/inc",
        "%{VendorPaths.drogon}/trantor",
        "%{EditorVendorDir}/jsoncpp/include",
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
        "drogon",

        "yaml-cpp",
        "angelscript",

        "ssl",
        "crypto",
        "crypt32",
    }

    defines {
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
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
            "z",
            "Ws2_32",
            "advapi32",
            "rpcrt4",
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
include "Magma/.deps/drogon"
