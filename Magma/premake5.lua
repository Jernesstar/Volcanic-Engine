project "Magma"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/lib")

    files {
        "src/Magma/**.h",
        "src/Magma/**.cpp"
    }

    includedirs {
        "src/Magma",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",
        "%{RootPath}/VolcaniCore/src/impl",

        "%{RootPath}/Magma/src",
        "%{RootPath}/Magma/src/Magma",

        "%{Includes.glm}",
        "%{Includes.glad}",
        "%{Includes.glfw}",

        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",

        "%{Includes.clay}",
        "%{Includes.flecs}",
        "%{Includes.PhysX}",

        "%{Includes.angelscript}",
        "%{MagmaVendorDir}",
        "%{Includes.soloud}",
        "%{Includes.asio}",
        "%{Includes.lmdb}"
    }

    links {
        "VolcaniCore",

        "yaml-cpp",

        "flecs",
        "angelscript",
        "soloud",
        "asio",
        "lmdb",

        "ssl",
        "crypto"
    }

    defines {
        "flecs_STATIC",
        "NOMINMAX",
        "YAML_CPP_STATIC_DEFINE"
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


include "Magma/.deps/angelscript"
include "Magma/.deps/soloud"
include "Magma/.deps/yaml-cpp"
include "Magma/.deps/glad"
include "Magma/.deps/asio"
include "Magma/.deps/lmdb"
