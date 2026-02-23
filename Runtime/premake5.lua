project "Runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    exceptionhandling "On"
    rtti "Off"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Runtime/obj")
    targetdir ("%{RootPath}/build/Runtime/bin")

    files {
        "src/Runtime/App/**.cpp",
        -- "src/**.cpp"
    }

    includedirs {
        "src",

        "%{RootPath}/VolcaniCore/**",

        "%{RootPath}/Engine/src",
        "%{RootPath}/Engine/src/**",

        "%{Includes.angelscript}",
        "%{VendorPaths.angelscript}",
        "%{LavaVendorDir}",

        "%{Includes.glm}",
    }

    links {
        "VolcaniCore",
        "Engine",

        "glfw",

        "angelscript",
        "flecs",
        "glad",
        "lmdb",
        "soloud",

        "z"
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
            "wayland-client"
        }

    filter "system:windows"
        systemversion "latest"
        links {
            "gdi32",
            "kernel32",
            "psapi",
            "Ws2_32",
            "stdc++exp"
        }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }
