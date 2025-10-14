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
include "MagmaServer"
-- include "Runtime"

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
VendorPaths["angelscript"]        = "%{MagmaVendorDir}/angelscript"
VendorPaths["yaml_cpp"]           = "%{MagmaVendorDir}/yaml-cpp"
VendorPaths["rapidjson"]          = "%{MagmaVendorDir}/rapidjson"

-- Lava libraries

-- Editor libraries
VendorPaths["drogon"]             = "%{EditorVendorDir}/drogon"
VendorPaths["jwt_cpp"]            = "%{EditorVendorDir}/jwt-cpp"
VendorPaths["stb_image"]          = "%{EditorVendorDir}/stb_image"
VendorPaths["soloud"]             = "%{EditorVendorDir}/soloud"
VendorPaths["efsw"]               = "%{EditorVendorDir}/efsw"
VendorPaths["libgit2"]            = "%{EditorVendorDir}/libgit2"
VendorPaths["miniz_cpp"]          = "%{EditorVendorDir}/miniz-cpp"
VendorPaths["glad"]               = "%{EditorVendorDir}/glad"
VendorPaths["clay"]               = "%{EditorVendorDir}/clay"
VendorPaths["imgui"]              = "%{EditorVendorDir}/imgui"
VendorPaths["ImGuiFileDialog"]    = "%{EditorVendorDir}/ImGuiFileDialog"
VendorPaths["ImGuizmo"]           = "%{EditorVendorDir}/ImGuizmo"
VendorPaths["ImGuiColorTextEdit"] = "%{EditorVendorDir}/ImGuiColorTextEdit"
VendorPaths["IconFontCppHeaders"] = "%{EditorVendorDir}/IconFontCppHeaders"

-- VolcaniCore libraries
Includes["glm"]                   = "%{VendorPaths.glm}"
Includes["glfw"]                  = "%{VendorPaths.glfw}/include"

-- Magma libraries
Includes["angelscript"]           = "%{VendorPaths.angelscript}/angelscript/include"
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include"

-- Lava libraries

-- Editor libraries
Includes["drogon"]                = "%{VendorPaths.drogon}/lib/inc"
Includes["jwt_cpp"]               = "%{VendorPaths.jwt_cpp}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["libgit2"]               = "%{VendorPaths.libgit2}/include"
Includes["miniz_cpp"]             = "%{VendorPaths.miniz_cpp}/include"
Includes["clay"]                  = "%{EditorVendorDir}"
Includes["imgui"]                 = "%{EditorVendorDir}"
Includes["ImGuiFileDialog"]       = "%{EditorVendorDir}"
Includes["ImGuizmo"]              = "%{EditorVendorDir}"
Includes["ImGuiColorTextEdit"]    = "%{EditorVendorDir}"
Includes["IconFontCppHeaders"]    = "%{EditorVendorDir}"