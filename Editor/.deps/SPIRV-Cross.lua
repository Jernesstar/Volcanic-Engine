project "SPIRV-Cross"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    files {
        "%{VendorPaths.SPIRV_Cross}/*.cpp",
    }

    removefiles {
        "%{VendorPaths.SPIRV_Cross}/main.cpp"
    }

    includedirs {
        "%{Includes.SPIRV_Cross}",
        "%{Includes.glslang}"
    }

    buildoptions {
        
    }