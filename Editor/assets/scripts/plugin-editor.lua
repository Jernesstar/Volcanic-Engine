
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

    _OPTIONS["VolcanicPaths"]
    includedirs {
    }

    links {
        "VolcaniCore"
    }

    defines {

    }