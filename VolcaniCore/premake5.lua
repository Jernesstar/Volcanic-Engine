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
        "src/VolcaniCore",

        "%{Includes.glm}",
    }

    defines {
        "GLM_FORCE_CXX20",
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
        }

        links {
            "stdc++exp"
        }
