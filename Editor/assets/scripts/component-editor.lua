project "${0}-Editor"
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"

    objdir ("%{ComponentPath}/Build/Platform/%{Target}/obj")
    targetdir ("%{ComponentPath}/Build/Platform/%{Target}/lib")

    files {
        -- "%{SourcePath}/Core/**.h",
        -- "%{SourcePath}/Core/**.cpp",
        "%{SourcePath}/Editor/**.h",
        "%{SourcePath}/Editor/**.cpp"
    }

    includedirs {
        "%{SourcePath}",
        "%{SourcePath}/Core",
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

    for _, dep in ipairs(CoreDeps) do
        links { dep }
    end

    for _, dep in ipairs(EditorDeps) do
        links { dep }
    end

    defines {

    }