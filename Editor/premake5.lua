
project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Editor/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Editor/bin")

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
        "%{Includes.imgui}",
        "%{Includes.imgui}/imgui",
        "%{Includes.ImGuiFileDialog}",
        "%{Includes.IconFontCppHeaders}",
    }

    links {
        "Lava",
        "Magma",
        "VolcaniCore",

        "glfw",

        "glad",
        "efsw",
        "libgit2",
        "soloud",
        "stb_image",
        "drogon",

        "imgui",
        "ImGuiFileDialog",
        "ImGuizmo",
        "ImGuiColorTextEdit",

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


include "Editor/.deps/glad"
include "Editor/.deps/soloud"
include "Editor/.deps/stb_image"
include "Editor/.deps/efsw"
include "Editor/.deps/libgit2"
include "Editor/.deps/drogon"
include "Editor/.deps/imgui"
include "Editor/.deps/ImGuiFileDialog"
include "Editor/.deps/ImGuizmo"
include "Editor/.deps/ImGuiColorTextEdit"
