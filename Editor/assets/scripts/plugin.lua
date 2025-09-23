workspace "${0}"
    location ("Build")
    architecture "x86_64"
    configurations { "Debug", "Release" }

    filter "system:linux"
        defines "VOLCANICENGINE_LINUX"

    filter "system:windows"
        defines {
            "VOLCANICENGINE_WINDOWS",
            "_DEBUG"
        }

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "Full"

    filter "action:vs*"
        startproject "Editor"

BuildPath = ${1};