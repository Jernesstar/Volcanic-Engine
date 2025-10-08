project "drogon"
    kind "StaticLib"
    language "C++"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Editor/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Editor/lib")

    files {
        "%{VendorPaths.drogon}/lib/src/**.h",
        "%{VendorPaths.drogon}/lib/src/**.cc",
        "%{VendorPaths.drogon}/trantor/**.h",
        "%{VendorPaths.drogon}/trantor/trantor/net/**.cc",
        "%{VendorPaths.drogon}/trantor/third_party/wepoll/wepoll.c",
        "%{EditorVendorDir}/jsoncpp/src/lib_json/**.h",
        "%{EditorVendorDir}/jsoncpp/src/lib_json/**.cpp",
        "%{EditorVendorDir}/c-ares/src/**.h",
        "%{EditorVendorDir}/c-ares/src/**.c",
    }

    removefiles {
        "%{VendorPaths.drogon}/trantor/trantor/utils/crypto/botan.cc",
        "%{VendorPaths.drogon}/trantor/trantor/net/inner/tlsprovider/BotanTLSProvider.cc",
        "%{VendorPaths.drogon}/trantor/trantor/unittests/**",
        "%{VendorPaths.drogon}/trantor/trantor/tests/**",
    }

    filter "system:windows"
        files {
            "%{VendorPaths.drogon}/third_party/mman-win32/mman.c",
        }

        removefiles {
            "%{VendorPaths.drogon}/trantor/trantor/net/inner/FileBufferNodeUnix.cc",
            "%{VendorPaths.drogon}/lib/src/SharedLibManager.cc",
        }

    filter "system:linux"
        removefiles {
            "%{VendorPaths.drogon}/trantor/trantor/net/inner/FileBufferNodeWin.cc",
        }

    filter { }

    includedirs {
        "%{VendorPaths.drogon}",
        "%{VendorPaths.drogon}/*",
        "%{VendorPaths.drogon}/**",
        "%{VendorPaths.drogon}/trantor",
        "%{VendorPaths.drogon}/trantor/*",
        "%{VendorPaths.drogon}/trantor/**",
        "%{EditorVendorDir}/jsoncpp/**",
        "%{EditorVendorDir}/c-ares/**",
    }

    defines {
        "DROGON_HAVE_STD_FILESYSTEM",
        "DROGON_HAVE_OPENSSL",
        "DROGON_HAVE_UNORDERED_MAP",
        "TRANTOR_USE_STD_FORMAT",
        "TRANTOR_USE_TLS=openssl",
        "USE_OPENSSL",
    }

    links {
        "ssl",
        "crypto",
    }

    filter "system:linux"
        buildoptions {
            "-mthreads"
        }

    filter "system:windows"
        defines {
            "_WIN32",
            "_WIN64",
            "_WIN32_WINDOWS",
        }

        links {
            "ws2_32",
            "mswsock"
        }