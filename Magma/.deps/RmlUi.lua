project "RmlUi"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    objdir ("%{RootPath}/build/Magma/obj")
    targetdir ("%{RootPath}/build/Magma/lib")

    files {
        "%{VendorPaths.RmlUi}/**.h",
        "%{VendorPaths.RmlUi}/Source/Core/**.cpp",
        "%{VendorPaths.RmlUi}/Backends/RmlUi_Platform_GLFW.cpp",
        "%{VendorPaths.RmlUi}/Backends/RmlUi_Renderer_GL3.cpp",
    }

    includedirs {
        "%{Includes.RmlUi}",
        "%{Includes.freetype}",
        "%{Includes.glfw}",
    }

    defines {
        "RMLUI_STATIC_LIB",
        "RMLUI_FONT_ENGINE_FREETYPE"
    }

    links {
        "freetype",
    }