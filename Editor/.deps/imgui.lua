project "imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Editor/obj")
    targetdir ("%{RootPath}/build/Editor/lib")

    files {
        "%{VendorPaths.imgui}/*.h",
        "%{VendorPaths.imgui}/*.cpp",

        "%{VendorPaths.imgui}/misc/cpp/imgui_stdlib.cpp",
        "%{VendorPaths.imgui}/backends/imgui_impl_glfw.cpp",
        "%{VendorPaths.imgui}/backends/imgui_impl_opengl3.cpp",
    }

    includedirs {
        "%{Includes.glfw}",
        "%{Includes.imgui}",
        "%{Includes.imgui}/imgui",
    }