project "VolcanicWindow"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/VolcanicWindow/obj")
    targetdir ("%{RootPath}/build/VolcanicWindow/lib")

    files {
        "src/VolcanicWindow/**.h",
        "src/VolcanicWindow/**.cpp",
    }

    includedirs {
        "src/",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/**",

        "%{Includes.glfw}",
        "%{Includes.glm}",
    }

    links {
        "glfw",
    }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }

    filter "system:windows"
        systemversion "latest"


include "VolcanicWindow/.deps/glfw"