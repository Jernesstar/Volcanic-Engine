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
        defines {
            "VOLCANIC_LINUX",
            -- "VOLCANIC_X11",
            "VOLCANIC_WAYLAND",
        }

    filter "system:apple"
        defines "VOLCANIC_APPLE"

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "Full"

    filter "action:vs*"
        startproject "Editor"

RootPath = _MAIN_SCRIPT_DIR;
VolcaniCoreVendorDir = "%{RootPath}/VolcaniCore/.vendor"
EngineVendorDir = "%{RootPath}/Engine/.vendor"
EditorVendorDir = "%{RootPath}/Editor/.vendor"

VendorPaths = {}
Includes = {}

-- Jolt physics config. This exact set of defines MUST be applied identically
-- to the Jolt static lib (Engine/.deps/Jolt.lua) AND every module that
-- includes Jolt headers (Engine, Editor). SIMD/config define mismatches
-- corrupt Jolt struct layout across the boundary. Keep in sync with the
-- -msse4.1/-msse4.2 build options in Jolt.lua.
JoltDefines = {
    "JPH_USE_SSE4_1",
    "JPH_USE_SSE4_2",
    -- Use plain aligned new/delete instead of Jolt's overridable allocator
    -- function-pointer hooks. This avoids a whole class of "allocator hook is
    -- null across the lib/consumer boundary" crashes and needs no runtime
    -- RegisterDefaultAllocator() to be reachable from consumer code.
    "JPH_DISABLE_CUSTOM_ALLOCATOR",
    -- CRITICAL: NDEBUG must be defined identically for the Jolt lib AND every
    -- consumer. Jolt derives JPH_DEBUG from !NDEBUG, and JPH_DEBUG changes its
    -- internal struct layout + enables asserts. A mismatch (lib built with
    -- JPH_DEBUG, Engine without) silently corrupts Jolt structs across the
    -- boundary and traps on the first allocation. The Engine/Editor projects
    -- already define NDEBUG; forcing it here keeps the lib in lockstep.
    "NDEBUG",
}

include "VolcaniCore"
include "Engine"
include "Editor"
include "Runtime"

-- VolcaniCore libraries
VendorPaths["glm"]                = "%{VolcaniCoreVendorDir}/glm"
VendorPaths["glfw"]               = "%{VolcaniCoreVendorDir}/glfw"
VendorPaths["spdlog"]             = "%{VolcaniCoreVendorDir}/spdlog"
-- Engine libraries
VendorPaths["angelscript"]        = "%{EngineVendorDir}/angelscript"
VendorPaths["soloud"]             = "%{EngineVendorDir}/soloud"
VendorPaths["glad"]               = "%{EngineVendorDir}/glad"
VendorPaths["flecs"]              = "%{EngineVendorDir}/flecs"
VendorPaths["lmdb"]               = "%{EngineVendorDir}/lmdb"
VendorPaths["libuv"]              = "%{EngineVendorDir}/libuv"
VendorPaths["uSockets"]           = "%{EngineVendorDir}/uSockets"
VendorPaths["uWebSockets"]        = "%{EngineVendorDir}/uWebSockets"
VendorPaths["Jolt"]               = "%{EngineVendorDir}/JoltPhysics"
-- Editor libraries
VendorPaths["imgui"]              = "%{EditorVendorDir}/imgui"
VendorPaths["ImGuizmo"]           = "%{EditorVendorDir}/ImGuizmo"
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
Includes["spdlog"]                = "%{VendorPaths.spdlog}/include"
-- Engine libraries
Includes["angelscript"]           = "%{VendorPaths.angelscript}/sdk/angelscript/include"
Includes["soloud"]                = "%{VendorPaths.soloud}/include"
Includes["glad"]                  = "%{VendorPaths.glad}/include"
Includes["flecs"]                 = "%{VendorPaths.flecs}/include"
Includes["lmdb"]                  = "%{VendorPaths.lmdb}/libraries/liblmdb"
Includes["uSockets"]              = "%{VendorPaths.uSockets}/src"
Includes["uWebSockets"]           = "%{EngineVendorDir}"
Includes["Jolt"]                  = "%{VendorPaths.Jolt}"
-- Editor libraries
Includes["imgui"]                 = "%{EditorVendorDir}"
Includes["ImGuizmo"]              = "%{EditorVendorDir}"
Includes["assimp"]                = "%{VendorPaths.assimp}/include"
Includes["SPIRV_Cross"]           = "%{VendorPaths.SPIRV_Cross}/include"
Includes["glslang"]               = "%{VendorPaths.glslang}"
Includes["yaml_cpp"]              = "%{VendorPaths.yaml_cpp}/include"
Includes["rapidjson"]             = "%{VendorPaths.rapidjson}/include"
Includes["stb_image"]             = "%{VendorPaths.stb_image}/include"
Includes["efsw"]                  = "%{VendorPaths.efsw}/include"
Includes["freetype"]              = "%{VendorPaths.freetype}/include"
