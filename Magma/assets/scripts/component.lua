-- newoption {
--     trigger     = "Target",
--     value       = "TARGET",
--     description = "Platform and compiler"
-- }

-- Target = _OPTIONS["Target"]

Target = "gcc-Windows"

workspace "${0}"
    location ("Platform/%{Target}")
    architecture "x86_64"
    configurations { "Debug", "Release" }

    defines {
        "_GLFW_USE_HYBRID_HPG"
    }

    filter "system:linux"
        defines {
            "VOLCANIC_LINUX",
            "_GLFW_X11"
        }

    filter "system:windows"
        defines {
            "VOLCANIC_WINDOWS",
            "_GLFW_WIN32",
        }

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"
        defines {
            "_DEBUG"
        }

    filter "configurations:Release"
        optimize "Full"

ComponentPath = "${1}"
SourcePath = "%{ComponentPath}/Source"
VendorPath = "%{ComponentPath}/Vendor"
VolcaniCorePath = "${2}"
MagmaPath = "${3}"

CoreDeps = {}
EditorDeps = {}
Defines = {}
VendorPaths = {}
