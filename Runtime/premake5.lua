project "Runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    exceptionhandling "On"
    rtti "Off"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Lava/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Lava/bin")

    files {
        "src/Runtime/**.cpp"
    }

    includedirs {
        "src",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",
        "%{RootPath}/VolcaniCore/src/impl",

        "%{RootPath}/Magma/src",
        "%{RootPath}/Magma/src/Magma",

        "%{RootPath}/Lava/src",
        "%{RootPath}/Lava/src/Lava",
        "%{RootPath}/Lava/src/Lava/**",

        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",
        
        "%{Includes.flecs}",
        "%{Includes.PhysX}",
        "%{Includes.clay}",
        "%{Includes.angelscript}",
        "%{VendorPaths.angelscript}",
        "%{Includes.soloud}",
        "%{Includes.asio}",

        "%{Includes.glm}",
        "%{Includes.glad}",
        "%{Includes.glfw}",
    }

    links {
        "Lava",
        "Magma",
        "VolcaniCore",

        "glfw",
        "glad",

        "yaml-cpp",

        "flecs",
        "angelscript",
        "soloud",
        "asio"
    }

    defines {
        "flecs_STATIC",
        "NOMINMAX",
    }

    filter "action:vs* or system:linux"
        links { "PhysX" }
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
