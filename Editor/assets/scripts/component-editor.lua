
project "AshEditor"
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
        "%{VolcaniCorePath}/*",
        "%{MagmaPath}",
        "%{MagmaPath}/*",
    }

    links {
        "VolcaniCore",
        "Magma"
    }

    defines {

    }