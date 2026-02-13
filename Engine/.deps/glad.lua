project "glad"
    kind "StaticLib"
    language "C"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Lava/obj")
    targetdir ("%{RootPath}/build/Lava/lib")

    files {
        "%{VendorPaths.glad}/src/glad.c"
    }

    includedirs {
        "%{Includes.glad}"
    }