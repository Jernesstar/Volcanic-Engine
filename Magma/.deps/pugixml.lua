project "pugixml"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Magma/obj")
    targetdir ("%{RootPath}/build/Magma/lib")

    files {
        "%{VendorPaths.pugixml}/src/*.cpp",
    }

    includedirs {
        "%{Includes.pugixml}",
    }

    buildoptions {
        "-Wunused-result"
    }