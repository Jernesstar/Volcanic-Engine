project "Lava"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Lava/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Lava/lib")

    files {
        "src/Lava/**.h",
        "src/Lava/**.cpp"
    }

    includedirs {
        "src/**",
        "src/Lava/",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",
        "%{RootPath}/VolcaniCore/src/impl",

        "%{RootPath}/Magma/src",
        "%{RootPath}/Magma/src/Magma",

        "%{Includes.PhysX}",
        "%{Includes.flecs}",
        "%{Includes.clay}",
        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",

        "%{Includes.glm}",
        "%{Includes.glad}",
        "%{Includes.glfw}",
        "%{Includes.imgui}",
        "%{Includes.angelscript}",
        "%{MagmaVendorDir}",
        "%{Includes.soloud}",
        "%{Includes.asio}",
        "%{Includes.lmdb}"
    }

    links {
        "Magma",
        "VolcaniCore",

        "yaml-cpp",
        "PhysX",
        "flecs",
        "asio",
        "lmdb",

        "ssl",
        "crypto"
    }

    defines {
        "flecs_STATIC",
        "NOMINMAX",
    }

    filter "action:vs* or system:linux"
        links { "PhysX" }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }

    filter "system:windows"
        systemversion "latest"

include "Lava/.deps/flecs"
include "Lava/.deps/PhysX"
