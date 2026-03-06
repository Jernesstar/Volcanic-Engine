rule "ASM"
    display "ASM Compile for angelscript"
    fileextension ".asm"
    location "%{RootPath}/build"

    buildmessage "Compiling %(Filename) ASM file for MSVC build"
    buildcommands 'ml64.exe /c /nologo /Fo"%(Outputs)as_callfunc_x64_msvc_asm.obj" /W3 /Zi /Ta "%(RootDir)%(Directory)%(Filename)%(Extension)"'
    buildoutputs '%(Outputs)as_callfunc_x64_msvc_asm.obj'

project "angelscript"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    rules { "ASM" }

    files {
        "%{VendorPaths.angelscript}/sdk/angelscript/source/*.cpp",
        "%{VendorPaths.angelscript}/sdk/add_on/**.cpp",
    }

    removefiles {
        "%{VendorPaths.angelscript}/sdk/add_on/autowrapper/**.cpp",
    }

    includedirs {
        "%{Includes.angelscript}",
        "%{VendorPaths.angelscript}",
    }

    defines {

    }

    filter { "action:vs*", "system:windows" }
        files {
            "%{VendorPaths.angelscript}/sdk/angelscript/source/as_callfunc_x64_msvc_asm.asm"
        }

        buildoptions {
            "/NODEFAULTLIB:library"
        }