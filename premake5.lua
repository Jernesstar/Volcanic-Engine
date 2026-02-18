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

include "Engine"
include "Editor"
include "Runtime"
include "RuntimeApp"
include "RuntimeWindow"

RootPath = _MAIN_SCRIPT_DIR;
VolcaniCoreVendorDir = "%{RootPath}/VolcaniCore/.vendor"
VolcanicWindowVendorDir = "%{RootPath}/VolcanicWindow/.vendor"
EngineVendorDir = "%{RootPath}/Engine/.vendor"
EditorVendorDir = "%{RootPath}/Editor/.vendor"

VendorPaths = {}
Includes = {}

-- VolcaniCore libraries
VendorPaths["glm"]  = "%{VolcaniCoreVendorDir}/glm"
-- VolcanicWindow libraries
VendorPaths["glfw"] = "%{VolcanicWindowVendorDir}/glfw"
-- Engine libraries
VendorPaths["angelscript"]        = "%{EngineVendorDir}/angelscript"
VendorPaths["soloud"]             = "%{EngineVendorDir}/soloud"
VendorPaths["glad"]               = "%{EngineVendorDir}/glad"
VendorPaths["flecs"]              = "%{EngineVendorDir}/flecs"
VendorPaths["lmdb"]               = "%{EngineVendorDir}/lmdb"
-- Editor libraries
VendorPaths["yaml_cpp"]           = "%{EngineVendorDir}/yaml-cpp"
VendorPaths["rapidjson"]          = "%{EngineVendorDir}/rapidjson"
VendorPaths["stb_image"]          = "%{EngineVendorDir}/stb_image"
VendorPaths["efsw"]               = "%{EngineVendorDir}/efsw"
VendorPaths["freetype"]           = "%{EngineVendorDir}/freetype"

-- VolcaniCore libraries
Includes["glm"]                   = "%{VendorPaths.glm}"
-- VolcanicWindow libraries
Includes["glfw"]                  = "%{VendorPaths.glfw}/include"
-- Engine libraries
Includes["angelscript"]           = "%{VendorPaths.angelscript}/angelscript/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["flecs"]                 = "%{VendorPaths.flecs}/distr"
Includes["lmdb"]                  = "%{VendorPaths.lmdb}/lmdb/libraries/liblmdb"
-- Editor libraries
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include"
Includes["json"]                  = "%{VendorPaths.json}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["freetype"]              = "%{VendorPaths.freetype}/include"