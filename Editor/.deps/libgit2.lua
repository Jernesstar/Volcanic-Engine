project "libgit2"
    kind "StaticLib"
    language "C"
    cdialect "C90"
    staticruntime "On"

    objdir ("%{RootPath}/build/%{_ACTION}/Editor/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Editor/lib")

    files {
        "%{VendorPaths.libgit2}/include/**.h",
        "%{VendorPaths.libgit2}/src/**.h",
        "%{VendorPaths.libgit2}/src/libgit2/*.c",
    }

    includedirs {
        "%{VendorPaths.libgit2}/include",
        "%{VendorPaths.libgit2}/include/**",
        "%{VendorPaths.libgit2}/src",
        "%{VendorPaths.libgit2}/src/**",
        "%{Includes.libgit2}",
    }

    defines {
        "LIBGIT2_NO_FEATURES_H",
        "__CLANG_INTTYPES_H"
    }

    filter "system:windows"
        files {
            "%{VendorPaths.libgit2}/src/libgit2/win32/*.c",
        }

    filter "system:linux"
        files {
            "%{VendorPaths.libgit2}/src/libgit2/unix/*.c",
        }