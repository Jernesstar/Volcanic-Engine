project "libgit2"
    kind "StaticLib"
    language "C"
    cdialect "C11"
    -- staticruntime "On"

    objdir ("%{RootPath}/build/Magma/obj")
    targetdir ("%{RootPath}/build/Magma/lib")

    files {
        "%{VendorPaths.libgit2}/include/**.h",
        "%{VendorPaths.libgit2}/src/**.h",
        "%{VendorPaths.libgit2}/src/util/*.c",
        "%{VendorPaths.libgit2}/src/util/allocators/*.c",
        "%{VendorPaths.libgit2}/src/util/hash/openssl.c",
        "%{VendorPaths.libgit2}/src/libgit2/*.c",
        "%{VendorPaths.libgit2}/src/libgit2/streams/*.c",
        "%{VendorPaths.libgit2}/src/libgit2/transports/*.c",
        "%{VendorPaths.libgit2}/deps/pcre/*.c",
        "%{VendorPaths.libgit2}/deps/llhttp/*.c",
        "%{VendorPaths.libgit2}/deps/xdiff/*.c",
    }

    includedirs {
        "%{VendorPaths.libgit2}",
        "%{VendorPaths.libgit2}/include",
        "%{VendorPaths.libgit2}/include/**",
        "%{VendorPaths.libgit2}/src",
        "%{VendorPaths.libgit2}/src/*",
        "%{VendorPaths.libgit2}/deps",
        "%{VendorPaths.libgit2}/deps/**",
        "%{Includes.libgit2}",
        "%{Includes.libgit2}/**",
    }

    defines {
        "_DEBUG",
        "_GNU_SOURCE",
        "GIT_USE_NSEC",
        "GIT_USE_STAT_MTIM",
        "GIT_TRACE",
        "_FILE_OFFSET_BITS=64",

        "HAVE_CONFIG_H",
        "GIT_REGEX_BUILTIN"
    }

    buildoptions {
        -- "-include stdint.h",
        "-Wall",
        "-Wextra",
        -- "-Wno-documentation",
        -- "-Wno-documentation-deprecated-sync",
        "-Wno-missing-field-initializers",
        "-Wmissing-declarations",
        "-Wstrict-aliasing",
        "-Wstrict-prototypes",
        "-Wdeclaration-after-statement",
        "-Wshift-count-overflow",
        "-Wunused-const-variable",
        "-Wunused-function",
        "-Wint-conversion",
        -- "-Wc11-extensions",
        "-Wc99-c11-compat",
        "-Wno-format",
        "-Wno-format-security",
    }

    filter "system:windows"
        files {
            "%{VendorPaths.libgit2}/src/libgit2/win32/*.c",
            "%{VendorPaths.libgit2}/src/util/win32/*.c",
        }

        defines {
            "WIN32",
            "_WIN32_WINNT=0x0600",
            "_CRT_SECURE_NO_WARNINGS",
            "WIN32_LEAN_AND_MEAN",
            "__USE_MINGW_ANSI_STDIO=1",

            "GIT_WINHTTP",
            "GIT_WIN32",
            "GIT_WIN32",

            "GIT_IO_WSAPOLL"
        }

    filter "system:linux"
        files {
            "%{VendorPaths.libgit2}/src/libgit2/unix/*.c",
            "%{VendorPaths.libgit2}/src/util/unix/*.c",
        }

        defines {
            "GIT_IO_POLL"
        }