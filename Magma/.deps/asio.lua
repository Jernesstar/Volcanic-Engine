project "asio"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/lib")

    files {
        "%{VendorPaths.asio}/asio/src/asio.cpp",
        "%{VendorPaths.asio}/asio/src/asio_ssl.cpp",
    }

    includedirs {
        "%{Includes.asio}",
        "%{Includes.asio}/asio/include",
    }

    defines {
        "_WIN32_WINDOWS",
        "ASIO_SEPARATE_COMPILATION"
    }

    buildoptions {
        "-mthreads"
    }