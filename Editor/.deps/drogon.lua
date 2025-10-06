project "drogon"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Editor/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Editor/lib")

    files {
        "%{VendorPaths.drogon}/drogon/src/drogon.cpp",
        "%{VendorPaths.drogon}/drogon/src/asio_ssl.cpp",
    }

    includedirs {
        "%{Includes.drogon}",
        "%{Includes.drogon}/drogon/include",
    }

    defines {
        "_WIN32_WINDOWS",
        -- "ASIO_STANDALONE",
        "ASIO_SEPARATE_COMPILATION",
    }

    filter "system:linux"
        buildoptions {
            "-mthreads"
        }

    links {
        "ssl",
        "crypto",
    }

    filter "system:windows"
        links {
            "ws2_32",
            "mswsock"
        }