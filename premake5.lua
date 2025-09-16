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
include "Editor"
include "Runtime"

RootPath = _MAIN_SCRIPT_DIR;
VolcaniCoreVendorDir = "%{RootPath}/VolcaniCore/.vendor"
MagmaVendorDir = "%{RootPath}/Magma/.vendor"
LavaVendorDir = "%{RootPath}/Lava/.vendor"
EditorVendorDir = "%{RootPath}/Editor/.vendor"
RuntimeVendorDir = "%{RootPath}/Runtime/.vendor"

VendorPaths = {}
Includes = {}

-- VolcaniCore libraries
VendorPaths["glm"]  = "%{VolcaniCoreVendorDir}/glm"
VendorPaths["glfw"] = "%{VolcaniCoreVendorDir}/glfw"

-- Magma libraries
VendorPaths["glad"]               = "%{MagmaVendorDir}/glad"
VendorPaths["angelscript"]        = "%{MagmaVendorDir}/angelscript"
VendorPaths["soloud"]             = "%{MagmaVendorDir}/soloud"
VendorPaths["yaml_cpp"]           = "%{MagmaVendorDir}/yaml-cpp"
VendorPaths["rapidjson"]          = "%{MagmaVendorDir}/rapidjson"
VendorPaths["asio"]               = "%{MagmaVendorDir}/asio"
VendorPaths["lmdb"]               = "%{MagmaVendorDir}/lmdb"

-- Lava libraries
VendorPaths["flecs"]              = "%{LavaVendorDir}/flecs"
VendorPaths["PhysX"]              = "%{LavaVendorDir}/PhysX"
VendorPaths["clay"]               = "%{LavaVendorDir}/clay"

-- Editor libraries
VendorPaths["imgui"]              = "%{EditorVendorDir}/imgui"
VendorPaths["ImGuiFileDialog"]    = "%{EditorVendorDir}/ImGuiFileDialog"
VendorPaths["ImGuizmo"]           = "%{EditorVendorDir}/ImGuizmo"
VendorPaths["ImGuiColorTextEdit"] = "%{EditorVendorDir}/ImGuiColorTextEdit"
VendorPaths["IconFontCppHeaders"] = "%{EditorVendorDir}/IconFontCppHeaders"
VendorPaths["assimp"]             = "%{EditorVendorDir}/assimp"
VendorPaths["freetype"]           = "%{EditorVendorDir}/freetype"
VendorPaths["stb_image"]          = "%{EditorVendorDir}/stb_image"
VendorPaths["glslang"]            = "%{EditorVendorDir}/glslang"
VendorPaths["SPIRV_Cross"]        = "%{EditorVendorDir}/SPIRV-Cross"
VendorPaths["efsw"]               = "%{EditorVendorDir}/efsw"
VendorPaths["miniz_cpp"]          = "%{EditorVendorDir}/miniz-cpp"

-- VolcaniCore libraries
Includes["glm"]                   = "%{VendorPaths.glm}"
Includes["glfw"]                  = "%{VendorPaths.glfw}/include"

-- Magma libraries
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["angelscript"]           = "%{VendorPaths.angelscript}/angelscript/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include"
Includes["asio"]                  = "%{VendorPaths.asio}/asio/include"

-- Lava libraries
Includes["flecs"]                 = "%{VendorPaths.flecs}/include"
Includes["PhysX"]                 = "%{VendorPaths.PhysX}/physx/include"
Includes["clay"]                  = "%{LavaVendorDir}"

-- Editor libraries
Includes["imgui"]                 = "%{EditorVendorDir}"
Includes["ImGuiFileDialog"]       = "%{EditorVendorDir}"
Includes["ImGuizmo"]              = "%{EditorVendorDir}"
Includes["ImGuiColorTextEdit"]    = "%{EditorVendorDir}"
Includes["IconFontCppHeaders"]    = "%{EditorVendorDir}"
Includes["assimp"]                = "%{VendorPaths.assimp}/include"
Includes["freetype"]              = "%{VendorPaths.freetype}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["glslang"]               = "%{VendorPaths.glslang}/glslang/Public"
Includes["SPIRV_Cross"]           = "%{VendorPaths.SPIRV_Cross}"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["miniz_cpp"]             = "%{VendorPaths.miniz_cpp}/include"