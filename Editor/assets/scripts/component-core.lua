project "${0}-Core"
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "Off"

    objdir ("%{ComponentPath}/Build/Platform/%{Target}/obj")
    targetdir ("%{ComponentPath}/Build/Platform/%{Target}/lib")

    files {
        "%{SourcePath}/Core/**.h",
        "%{SourcePath}/Core/**.cpp"
    }

    includedirs {
        "%{SourcePath}",
        "%{SourcePath}/**",
        "%{VolcaniCorePath}",
        "%{VolcaniCorePath}/**",
        "%{VolcaniCorePath}/../.vendor/glm",
        "%{MagmaPath}",
        "%{MagmaPath}/**",
    }

    for name, path in pairs(VendorPaths) do
        includedirs {
            path,
            path .. "/*",
            path .. "/*/*"
        }
    end

    libdirs {
        -- "%{VolcaniCorePath}/../../lib"
        "%{VolcaniCorePath}/../../build/**",
    }

    links {
        "VolcaniCore",
        "Magma"
    }

    for i, dep in ipairs(CoreDeps) do
        links { dep }
    end

    defines {

    }

    for i, def in ipairs(Defines) do
        defines { def }
    end

    filter "system:windows"
        systemversion "latest"