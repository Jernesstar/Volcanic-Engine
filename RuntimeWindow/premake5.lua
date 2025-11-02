project "RuntimeWindow"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    exceptionhandling "On"
    rtti "Off"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Lava/obj")
    targetdir ("%{RootPath}/build/Lava/bin")

    files {
        "src/**.cpp"
    }

    includedirs {
        "src",

        "%{RootPath}/VolcaniCore/**",
        "%{RootPath}/VolcanicWindow/**",

        "%{RootPath}/Lava/src",
        "%{RootPath}/Lava/src/Lava",
        "%{RootPath}/Lava/src/Lava/**",

        "%{Includes.angelscript}",
        "%{VendorPaths.angelscript}",
        "%{LavaVendorDir}",

        "%{Includes.glm}",
        "%{Includes.glfw}",
    }

    links {
        "Lava",
        "VolcaniCore",
        "VolcanicWindow",
        "glfw",
        "angelscript",
    }

    defines {
        "NOMINMAX",
    }

    filter "action:vs* or system:linux"
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
