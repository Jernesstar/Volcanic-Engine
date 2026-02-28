project "flecs"
    kind "StaticLib"
    language "C"
    cdialect "C99"
    staticruntime "Off"

    objdir ("%{RootPath}/build/Engine/obj")
    targetdir ("%{RootPath}/build/Engine/lib")

    files {
        "%{VendorPaths.flecs}/distr/flecs.h",
        "%{VendorPaths.flecs}/distr/flecs.c",
    }

    includedirs {
        "%{VendorPaths.flecs}/distr",
    }

    defines {
        -- "flecs_EXPORTS",
        "FLECS_STATIC",
        "FLECS_DEBUG",
        -- "FLECS_NDEBUG",
        -- "FLECS_CPP_NO_AUTO_REGISTRATION",

        -- "FLECS_CUSTOM_BUILD",
        -- "FLECS_CPP",
        -- "FLECS_ALERTS",
        -- "FLECS_SYSTEM",
        -- "FLECS_PIPELINE",
        -- "FLECS_TIMER",
        -- "FLECS_LOG",
        -- "FLECS_METRICS",
        -- "FLECS_STATS",
    }

    filter "toolset:gcc"
       cdialect "gnu99"