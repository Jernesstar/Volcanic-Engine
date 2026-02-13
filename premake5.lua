workspace "VolcanicEngine"
    location ("build")
    architecture "x86_64"
    configurations { "Debug", "Release" }

    filter "system:windows"
        defines {
            "VOLCANIC_WINDOWS",
            "_DEBUG",
            "_WIN32",
            "_WIN64",
        }

    filter "system:linux"
        defines "VOLCANIC_LINUX"

    filter "system:apple"
        defines "VOLCANIC_APPLE"

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "Full"

    filter "action:vs*"
        startproject "Editor"

include "VolcaniCore"
include "VolcanicWindow"
include "Lava"
include "Runtime"
include "RuntimeWindow"

RootPath = _MAIN_SCRIPT_DIR;
VolcaniCoreVendorDir = "%{RootPath}/VolcaniCore/.vendor"
VolcanicWindowVendorDir = "%{RootPath}/VolcanicWindow/.vendor"
LavaVendorDir = "%{RootPath}/Lava/.vendor"

VendorPaths = {}
Includes = {}

-- VolcaniCore libraries
VendorPaths["glm"]  = "%{VolcaniCoreVendorDir}/glm"

-- VolcanicWindow libraries
VendorPaths["glfw"] = "%{VolcanicWindowVendorDir}/glfw"

-- Lava libraries
VendorPaths["angelscript"]        = "%{LavaVendorDir}/angelscript"
VendorPaths["yaml_cpp"]           = "%{LavaVendorDir}/yaml-cpp"
VendorPaths["rapidjson"]          = "%{LavaVendorDir}/rapidjson"
VendorPaths["stb_image"]          = "%{LavaVendorDir}/stb_image"
VendorPaths["soloud"]             = "%{LavaVendorDir}/soloud"
VendorPaths["efsw"]               = "%{LavaVendorDir}/efsw"
VendorPaths["glad"]               = "%{LavaVendorDir}/glad"
VendorPaths["freetype"]           = "%{LavaVendorDir}/freetype"

-- VolcaniCore libraries
Includes["glm"]                   = "%{VendorPaths.glm}"

-- VolcanicWindow libraries
Includes["glfw"]                  = "%{VendorPaths.glfw}/include"

-- Lava libraries
Includes["angelscript"]           = "%{VendorPaths.angelscript}/angelscript/include"
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include" 
Includes["json"]                  = "%{VendorPaths.json}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["freetype"]              = "%{VendorPaths.freetype}/include"