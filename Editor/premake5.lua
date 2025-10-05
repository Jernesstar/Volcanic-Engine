
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
        "%{Includes.asio}",
        "%{Includes.stb_image}",
        "%{Includes.soloud}",
        "%{Includes.miniz_cpp}",
        "%{Includes.ai_sdk_cpp}",
        "%{Includes.ai_sdk_cpp}/**",
        "%{VendorPaths.ai_sdk_cpp}/third_party/*/include",
    
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
        "soloud",
        "stb_image",
        "asio",
        "ai-sdk-cpp",

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
        "YAML_CPP_STATIC_DEFINE",
        "AI_SDK_HAS_OPENAI",
        "AI_SDK_HAS_ANTHROPIC",
        "CPPHTTPLIB_OPENSSL_SUPPORT",
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


include "Editor/.deps/glad"
include "Editor/.deps/soloud"
include "Editor/.deps/stb_image"
include "Editor/.deps/efsw"
include "Editor/.deps/asio"
include "Editor/.deps/ai-sdk-cpp"
include "Editor/.deps/imgui"
include "Editor/.deps/ImGuiFileDialog"
include "Editor/.deps/ImGuizmo"
include "Editor/.deps/ImGuiColorTextEdit"
