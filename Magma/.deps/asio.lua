project "asio"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Magma/obj")
    targetdir ("%{RootPath}/build/Magma/lib")

    files {
        "%{VendorPaths.asio}/src/asio.cpp",
        "%{VendorPaths.asio}/src/asio_ssl.cpp",
    }

    includedirs {
        "%{Includes.asio}",
    }

    defines {
        -- "ASIO_STANDALONE",
        "ASIO_SEPARATE_COMPILATION",
    }

    links {
        "ssl",
        "crypto",
        "pthreads",
    }

    filter "system:windows"
        links {
            "ws2_32",
            "mswsock"
        }

        defines {
            "_WIN32_WINDOWS",
        }