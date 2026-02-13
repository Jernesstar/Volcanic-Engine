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
MagmaVendorDir = "%{RootPath}/Magma/.vendor"

VendorPaths = {}
Includes = {}

-- VolcaniCore libraries
VendorPaths["glm"]  = "%{VolcaniCoreVendorDir}/glm"

-- VolcanicWindow libraries
VendorPaths["glfw"] = "%{VolcanicWindowVendorDir}/glfw"

-- Lava libraries
VendorPaths["angelscript"]        = "%{LavaVendorDir}/angelscript"

-- Magma libraries
VendorPaths["pugixml"]            = "%{MagmaVendorDir}/pugixml"
VendorPaths["yaml_cpp"]           = "%{MagmaVendorDir}/yaml-cpp"
VendorPaths["rapidjson"]          = "%{MagmaVendorDir}/rapidjson"
VendorPaths["json"]               = "%{MagmaVendorDir}/json"
VendorPaths["cpp_httplib"]        = "%{MagmaVendorDir}/cpp-httplib"
VendorPaths["stb_image"]          = "%{MagmaVendorDir}/stb_image"
VendorPaths["soloud"]             = "%{MagmaVendorDir}/soloud"
VendorPaths["efsw"]               = "%{MagmaVendorDir}/efsw"
VendorPaths["libgit2"]            = "%{MagmaVendorDir}/libgit2"
VendorPaths["miniz_cpp"]          = "%{MagmaVendorDir}/miniz-cpp"
VendorPaths["glad"]               = "%{MagmaVendorDir}/glad"
VendorPaths["freetype"]           = "%{MagmaVendorDir}/freetype"
VendorPaths["RmlUi"]              = "%{MagmaVendorDir}/RmlUi"

-- VolcaniCore libraries
Includes["glm"]                   = "%{VendorPaths.glm}"

-- VolcanicWindow libraries
Includes["glfw"]                  = "%{VendorPaths.glfw}/include"

-- Lava libraries
Includes["angelscript"]           = "%{VendorPaths.angelscript}/angelscript/include"

-- Magma libraries
Includes["pugixml"]               = "%{VendorPaths.pugixml}/src"
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include"
Includes["json"]                  = "%{VendorPaths.json}/include"
Includes["cpp_httplib"]           = "%{MagmaVendorDir}"
Includes["jwt_cpp"]               = "%{VendorPaths.jwt_cpp}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["libgit2"]               = "%{VendorPaths.libgit2}/include"
Includes["miniz_cpp"]             = "%{VendorPaths.miniz_cpp}/include"
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["freetype"]              = "%{VendorPaths.freetype}/include"
Includes["RmlUi"]                 = "%{VendorPaths.RmlUi}/Include"