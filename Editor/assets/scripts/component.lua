workspace "${0}"
    location ("Build")
    architecture "x86_64"
    configurations { "Debug", "Release" }

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "Full"

BuildPath = ${1};
VolcaniCorePath = ${2};
MagmaPath = ${3};