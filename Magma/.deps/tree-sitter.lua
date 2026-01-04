project "tree-sitter"
    kind "StaticLib"
    language "C"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Magma/obj")
    targetdir ("%{RootPath}/build/Magma/lib")

    files {
        "%{VendorPaths.tree-sitter}/src/tree-sitter.c"
    }

    includedirs {
        "%{Includes.tree-sitter}"
    }