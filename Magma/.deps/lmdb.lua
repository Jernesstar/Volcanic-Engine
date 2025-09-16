project "lmdb"
    kind "StaticLib"
    language "C"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/lib")

    files {
        "%{VendorPaths.lmdb}/"
    }

    includedirs {
        "%{Includes.lmdb}",
    }

    defines {
        ""
    }

    buildoptions {
        -- "-Wunused-result"
    }