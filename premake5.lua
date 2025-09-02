workspace "VolcanicEngine"
    location ("build/%{_ACTION}")
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

    filter "action:vs*"
        startproject "Editor"

include "VolcaniCore"
include "Magma"
include "Lava"

RootPath = _MAIN_SCRIPT_DIR;
VolcaniCoreVendorDir = "%{RootPath}/VolcaniCore/vendor"
MagmaVendorDir = "%{RootPath}/Magma/vendor"
LavaVendorDir = "%{RootPath}/Lava/vendor"

VendorPaths = {}
Includes = {}

-- VolcaniCore libraries
VendorPaths["glm"]  = "%{VolcaniCoreVendorDir}/glm"
VendorPaths["glfw"] = "%{VolcaniCoreVendorDir}/glfw"

-- Magma libraries
VendorPaths["glad"]               = "%{MagmaVendorDir}/glad"
VendorPaths["angelscript"]        = "%{MagmaVendorDir}/angelscript"
VendorPaths["soloud"]             = "%{MagmaVendorDir}/soloud"
VendorPaths["freetype"]           = "%{MagmaVendorDir}/freetype"
VendorPaths["yaml_cpp"]           = "%{MagmaVendorDir}/yaml-cpp"
VendorPaths["rapidjson"]          = "%{MagmaVendorDir}/rapidjson"

-- Lava libraries
VendorPaths["flecs"]              = "%{LavaVendorDir}/flecs"
VendorPaths["PhysX"]              = "%{LavaVendorDir}/PhysX"
VendorPaths["clay"]               = "%{LavaVendorDir}/clay"

-- Editor libraries
VendorPaths["imgui"]              = "%{MagmaVendorDir}/imgui"
VendorPaths["ImGuiFileDialog"]    = "%{MagmaVendorDir}/ImGuiFileDialog"
VendorPaths["ImGuizmo"]           = "%{MagmaVendorDir}/ImGuizmo"
VendorPaths["ImGuiColorTextEdit"] = "%{MagmaVendorDir}/ImGuiColorTextEdit"
VendorPaths["IconFontCppHeaders"] = "%{MagmaVendorDir}/IconFontCppHeaders"
VendorPaths["assimp"]             = "%{MagmaVendorDir}/assimp"
VendorPaths["stb_image"]          = "%{MagmaVendorDir}/stb_image"
VendorPaths["efsw"]               = "%{MagmaVendorDir}/efsw"
VendorPaths["glslang"]            = "%{MagmaVendorDir}/glslang"
VendorPaths["SPIRV_Cross"]        = "%{MagmaVendorDir}/SPIRV-Cross"

-- VolcaniCore libraries
Includes["glm"]                   = "%{VendorPaths.glm}"
Includes["glfw"]                  = "%{VendorPaths.glfw}/include"

-- Magma libraries
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["angelscript"]           = "%{VendorPaths.angelscript}/angelscript/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["freetype"]              = "%{VendorPaths.freetype}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include"
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"

-- Lava libraries
Includes["flecs"]                 = "%{VendorPaths.flecs}/include"
Includes["PhysX"]                 = "%{VendorPaths.PhysX}/physx/include"
Includes["clay"]                  = "%{LavaVendorDir}"

-- Editor libraries
Includes["imgui"]                 = "%{MagmaVendorDir}"
Includes["ImGuiFileDialog"]       = "%{MagmaVendorDir}"
Includes["ImGuizmo"]              = "%{MagmaVendorDir}"
Includes["ImGuiColorTextEdit"]    = "%{MagmaVendorDir}"
Includes["IconFontCppHeaders"]    = "%{MagmaVendorDir}"
Includes["assimp"]                = "%{VendorPaths.assimp}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["glslang"]               = "%{VendorPaths.glslang}/glslang/Public"
Includes["SPIRV_Cross"]           = "%{VendorPaths.SPIRV_Cross}"
