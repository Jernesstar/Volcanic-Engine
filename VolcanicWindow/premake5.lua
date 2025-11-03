project "VolcanicWindow"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/VolcanicWindow/obj")
    targetdir ("%{RootPath}/build/VolcanicWindow/lib")

    files {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs {
        "src/",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/*",

        "%{Includes.glm}",
        "%{Includes.glfw}",
    }

    links {
        "glfw",
    }

    buildoptions {
        "-Wno-format-security",
        "-Wno-pointer-arith",
        "-Wno-template-body"
    }

    filter "system:windows"
        systemversion "latest"


include "VolcanicWindow/.deps/glfw"