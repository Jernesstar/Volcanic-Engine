project "ai-sdk-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "Off"

    objdir ("%{RootPath}/build/%{_ACTION}/Magma/obj")
    targetdir ("%{RootPath}/build/%{_ACTION}/Magma/lib")

    files {
        "%{VendorPaths.ai_sdk_cpp}/src/*.h",
        "%{VendorPaths.ai_sdk_cpp}/src/**.cpp",
    }

    includedirs {
        "%{Includes.ai_sdk_cpp}",
        "%{Includes.ai_sdk_cpp}/**",
        "%{VendorPaths.ai_sdk_cpp}/src",
        "%{VendorPaths.ai_sdk_cpp}/src/**",
        "%{VendorPaths.ai_sdk_cpp}/third_party",
        "%{VendorPaths.ai_sdk_cpp}/third_party/*",
        "%{VendorPaths.ai_sdk_cpp}/third_party/*/include",
    }

    defines {
        "AI_SDK_HAS_OPENAI",
        "AI_SDK_HAS_ANTHROPIC",
        "CPPHTTPLIB_OPENSSL_SUPPORT",
    }
