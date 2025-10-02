project "${0}-Core"
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "Off"

    objdir ("%{ComponentPath}/Build/Platform/%{Target}/obj")
    targetdir ("%{ComponentPath}/Build/Platform/%{Target}/lib")

    files {
        "%{SourcePath}/*.h",
        "%{SourcePath}/*.cpp",
        "%{SourcePath}/Core/**.h",
        "%{SourcePath}/Core/**.cpp"
    }

    includedirs {
        "%{SourcePath}",
        "%{SourcePath}/**",
        "%{VolcaniCorePath}",
        "%{VolcaniCorePath}/**",
        "%{VolcaniCorePath}/../.vendor/glm",
        "%{VolcaniCorePath}/../.vendor/glfw/include",
        "%{MagmaPath}",
        "%{MagmaPath}/**",
        "%{MagmaPath}/../.vendor/angelscript/angelscript/include",
        "%{MagmaPath}/../../Lava/src",
        "%{VendorPath}"
    }

    for name, path in pairs(VendorPaths) do
        includedirs {
            path,
            path .. "/include",
        }
    end

    libdirs {
        "%{VolcaniCorePath}/../../build/**",
    }

    links {
        "VolcaniCore",
        "Magma",
        "angelscript",
        "glfw"
    }

    filter "system:linux"
        links {
            "pthread",
            "dl",
            "GL",
            "X11",
        }

    filter "system:windows"
        systemversion "latest"
        links {
            "gdi32",
            "kernel32",
            "psapi",
            "Ws2_32",
        }

    for i, dep in ipairs(CoreDeps) do
        links { dep }
    end

    defines {

    }

    for i, def in ipairs(Defines) do
        defines { def }
    end