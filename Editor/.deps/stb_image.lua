project "stb_image"
    kind "StaticLib"
    language "C"

    objdir ("%{RootPath}/build/Editor/obj")
    targetdir ("%{RootPath}/build/Editor/lib")

    files {
        "%{VendorPaths.stb_image}/include/stb_image/stb_image.h",
        "%{VendorPaths.stb_image}/src/stb_image.cpp",
    }

    includedirs {
        "%{Includes.stb_image}"
    }
