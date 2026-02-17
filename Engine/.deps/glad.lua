project "glad"
    kind "StaticLib"
    language "C"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    files {
        "%{VendorPaths.glad}/src/glad.c"
    }

    includedirs {
        "%{Includes.glad}"
    }