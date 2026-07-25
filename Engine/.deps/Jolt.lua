project "Jolt"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"
    exceptionhandling "On"
    rtti "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    files {
        "%{VendorPaths.Jolt}/Jolt/**.cpp",
        "%{VendorPaths.Jolt}/Jolt/**.h",
    }

    includedirs {
        "%{Includes.Jolt}",
    }

    -- These MUST match JoltDefines used by every consumer (see premake5.lua).
    -- A mismatch in the SIMD/config defines silently corrupts Jolt's struct
    -- layout across the library/consumer boundary.
    defines { JoltDefines }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-msse4.1",
            "-msse4.2",
            "-mpopcnt",
        }
