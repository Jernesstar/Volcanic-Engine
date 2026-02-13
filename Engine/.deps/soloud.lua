project "soloud"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Lava/obj")
    targetdir ("%{RootPath}/build/Lava/lib")

    files {
        "%{VendorPaths.soloud}/src/core/*.cpp",
        "%{VendorPaths.soloud}/src/backend/miniaudio/*.cpp",
        "%{VendorPaths.soloud}/src/audiosource/wav/*",
    }

    includedirs {
        "%{Includes.soloud}",
    }

    defines {
        "WITH_MINIAUDIO"
    }

    buildoptions {
        -- "-Wunused-result"
    }