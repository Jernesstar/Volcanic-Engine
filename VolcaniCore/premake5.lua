project "VolcaniCore"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/VolcaniCore/obj")
    targetdir ("%{RootPath}/build/VolcaniCore/lib")

    files {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs {
        "src/",
        "src/**",

        "%{Includes.glm}",
        "%{Includes.glfw}",
        "%{Includes.spdlog}",
    }

    defines {
        "GLM_FORCE_CXX20",
        "NOMINMAX",
        "NDEBUG",
        "WIN32_LEAN_AND_MEAN",
    }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }

    filter "system:windows"
        systemversion "latest"

        defines {
            "GLM_FORCE_PLATFORM_WIN32",
            "GLM_FORCE_SILENT_WARNINGS"
        }

        buildoptions {
            "-Wno-error=template-body",
        }

        links {
            "stdc++exp"
        }

include "VolcaniCore/.deps/glfw"