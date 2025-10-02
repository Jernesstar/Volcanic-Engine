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

        "%{Includes.yaml_cpp}",
        "%{Includes.rapidjson}",

        "%{Includes.glm}",
        "%{Includes.glfw}",
        "%{Includes.angelscript}",
        "%{MagmaVendorDir}",
    }

    links {
        "Magma",
        "VolcaniCore",

        -- "yaml-cpp",
    }

    defines {
        "NOMINMAX",
    }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }

    filter "system:windows"
        systemversion "latest"
