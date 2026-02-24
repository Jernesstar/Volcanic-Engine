workspace "VolcanicEngine"
    location ("build")
    architecture "x86_64"
    configurations { "Debug", "Release" }

    filter "system:windows"
        defines {
            "VOLCANIC_WINDOWS",
            "_WIN32",
            "_WIN64",
            "NDEBUG"
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
include "Engine"
include "Editor"
include "Runtime"

RootPath = _MAIN_SCRIPT_DIR;
VolcaniCoreVendorDir = "%{RootPath}/VolcaniCore/.vendor"
EngineVendorDir = "%{RootPath}/Engine/.vendor"
EditorVendorDir = "%{RootPath}/Editor/.vendor"

VendorPaths = {}
Includes = {}

-- VolcaniCore libraries
VendorPaths["glm"]                = "%{VolcaniCoreVendorDir}/glm"
VendorPaths["glfw"]               = "%{VolcaniCoreVendorDir}/glfw"
-- Engine libraries
VendorPaths["angelscript"]        = "%{EngineVendorDir}/angelscript"
VendorPaths["soloud"]             = "%{EngineVendorDir}/soloud"
VendorPaths["glad"]               = "%{EngineVendorDir}/glad"
VendorPaths["flecs"]              = "%{EngineVendorDir}/flecs"
VendorPaths["lmdb"]               = "%{EngineVendorDir}/lmdb"
-- Editor libraries
VendorPaths["assimp"]             = "%{EditorVendorDir}/assimp"
VendorPaths["glslang"]            = "%{EditorVendorDir}/glslang"
VendorPaths["SPIRV_Cross"]        = "%{EditorVendorDir}/SPIRV-Cross"
VendorPaths["yaml_cpp"]           = "%{EditorVendorDir}/yaml-cpp"
VendorPaths["rapidjson"]          = "%{EditorVendorDir}/rapidjson"
VendorPaths["stb_image"]          = "%{EditorVendorDir}/stb_image"
VendorPaths["efsw"]               = "%{EditorVendorDir}/efsw"
VendorPaths["freetype"]           = "%{EditorVendorDir}/freetype"

-- VolcaniCore libraries
Includes["glm"]                   = "%{VendorPaths.glm}"
Includes["glfw"]                  = "%{VendorPaths.glfw}/include"
-- Engine libraries
Includes["angelscript"]           = "%{VendorPaths.angelscript}/angelscript/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["flecs"]                 = "%{VendorPaths.flecs}/distr"
Includes["lmdb"]                  = "%{VendorPaths.lmdb}/lmdb/libraries/liblmdb"
-- Editor libraries
Includes["assimp"]                = "%{VendorPaths.assimp}/include"
Includes["SPIRV_Cross"]           = "%{VendorPaths.SPIRV_Cross}/include"
Includes["glslang"]               = "%{VendorPaths.glslang}/include"
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include"
Includes["json"]                  = "%{VendorPaths.json}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["freetype"]              = "%{VendorPaths.freetype}/include"