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

    filter "system:linux"
        defines "VOLCANICENGINE_LINUX"

    filter "system:windows"
        defines {
            "VOLCANICENGINE_WINDOWS",
            "_DEBUG"
        }

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"

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
