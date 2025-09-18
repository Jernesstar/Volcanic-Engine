project "soloud"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Editor/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Editor/lib")

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