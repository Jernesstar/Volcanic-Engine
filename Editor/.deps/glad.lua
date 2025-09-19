project "glad"
    kind "StaticLib"
    language "C"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Editor/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Editor/lib")

    files {
        "%{VendorPaths.glad}/src/glad.c"
    }

    includedirs {
        "%{Includes.glad}"
    }