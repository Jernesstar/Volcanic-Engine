project "MagmaServer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    exceptionhandling "On"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Server/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Server/bin")

    files {
        "**.h",
        "**.cpp"
    }

    includedirs {
        "src",

        "%{RootPath}/VolcaniCore/src",
        "%{RootPath}/VolcaniCore/src/VolcaniCore",
        "%{RootPath}/VolcaniCore/src/impl",

        "%{Includes.glm}",
        "%{Includes.glfw}",

        "%{Includes.drogon}",
        "%{VendorPaths.drogon}/*",
        "%{VendorPaths.drogon}/orm_lib/inc",
        "%{VendorPaths.drogon}/nosql_lib/redis/inc",
        "%{VendorPaths.drogon}/trantor",
        "%{EditorVendorDir}/jsoncpp/include",
        "%{Includes.jwt_cpp}",
    }

    links {
        "VolcaniCore",

        "glfw",
        "drogon",

        "ssl",
        "crypto",
        "crypt32",
    }

    defines {
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
        "YAML_CPP_STATIC_DEFINE"
    }

    filter "toolset:msc or system:linux"
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
            "z",
            "Ws2_32",
            "advapi32",
            "rpcrt4",
        }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-fexceptions",
            "-Wno-format-security",
            "-Wno-pointer-arith"
        }
