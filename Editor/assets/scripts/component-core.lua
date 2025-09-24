project "${0}-Core"
    kind "DynamicLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "Off"

    objdir ("Build/Platform/%{Target}/obj")
    targetdir ("Build/Platform/%{Target}/lib")

    files {
        "%{SourcePath}/Core/**.h",
        "%{SourcePath}/Core/**.cpp"
    }

    includedirs {
        "%{SourcePath}",
        "%{SourcePath}/Core",
        "%{VolcaniCorePath}",
        "%{VolcaniCorePath}/**",
        "%{MagmaPath}",
        "%{MagmaPath}/**",
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
