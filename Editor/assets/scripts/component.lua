-- newoption {
--     trigger     = "Target",
--     value       = "TARGET",
--     description = "Platform and compiler"
-- }

-- Target = _OPTIONS["Target"]

workspace "${0}"
    location ("Platform/%{Target}")
    architecture "x86_64"
    configurations { "Debug", "Release" }

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

Target = "gcc-Windows"

CoreDeps = {}
EditorDeps = {}
Defines = {}
VendorPaths = {}
