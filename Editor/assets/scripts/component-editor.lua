project "${0}-Editor"
    kind "DynamicLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("Build/%{Target}/obj")
    targetdir ("Build/%{Target}/lib")

    files {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs {
        "%{VolcaniCorePath}",
        "%{VolcaniCorePath}/**",
        "%{MagmaPath}",
        "%{MagmaPath}/**",
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