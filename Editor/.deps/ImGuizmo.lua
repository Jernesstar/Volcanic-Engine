project "ImGuizmo"
    kind "StaticLib"
    language "C++"

    objdir ("%{RootPath}/build/Editor/obj")
    targetdir ("%{RootPath}/build/Editor/lib")

    files {
        "%{VendorPaths.ImGuizmo}/*.h",
        "%{VendorPaths.ImGuizmo}/*.cpp"
    }

    includedirs {
        "%{Includes.ImGuizmo}",
        "%{Includes.imgui}/imgui"
    }