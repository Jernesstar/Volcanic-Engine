project "uSockets"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    files {
        "%{VendorPaths.uSockets}/src/*.c",
        "%{VendorPaths.uSockets}/src/eventing/libuv.c",
        "%{VendorPaths.uSockets}/src/crypto/openssl.c",
        "%{VendorPaths.uSockets}/src/crypto/sni_tree.cpp",
    }

    includedirs {
        "%{VendorPaths.uSockets}",
        "%{VendorPaths.uSockets}/src",
        "%{VendorPaths.libuv}/include",
    }

    defines {
        "LIBUS_USE_LIBUV",
        "LIBUS_USE_OPENSSL",
    }

if os.target() == "windows" and _ACTION == "gmake" then
project "uv"
    kind "StaticLib"
    language "C"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/bin")

    -- Include directories
    includedirs {
        "%{VendorPaths.libuv}/include",
        "%{VendorPaths.libuv}/src"
    }

    -- Defines for Windows/MinGW
    defines {
        "WIN32",
        "_WIN32",
        "_WIN32_WINNT=0x0A00", -- Windows Vista or higher
        "__MINGW32__"
    }

    -- Files based on libuv structure
    files {
        "%{VendorPaths.libuv}/include/**.h",
        "%{VendorPaths.libuv}/src/*.c",
        "%{VendorPaths.libuv}/src/win/*.c",
        "%{VendorPaths.libuv}/src/win/*.h"
    }

    -- Exclude non-win files if necessary
    removefiles {
        "src/unix/**"
    }

    filter "platforms:x64"
        defines { "_AMD64_" }

    filter "platforms:x86"
        defines { "_X86_" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
end
