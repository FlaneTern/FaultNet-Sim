workspace "FaultNet-Sim"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    architecture "x64"

    startproject "FaultNet-Sim"


OutputDir = "%{cfg.buildcfg}-%{cfg.architecture}"

project "FaultNet-Sim"
    location "FaultNet-Sim"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    pchheader "PCH.h"
    pchsource "%{prj.name}/source/PCH.cpp"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("bin-itm/" .. OutputDir .. "/%{prj.name}")

    files {
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.cpp",
        "%{prj.name}/interface/**.h",
        "%{prj.name}/interface/**.cpp",
        "%{prj.name}/FaultNet-Sim.h"
    }

    includedirs { 
        "%{prj.name}",
        "%{prj.name}/source",
        "%{prj.name}/interface",
        "Dependencies/SQLite/sqlite-amalgamation-3460100"
     }

    systemversion "latest"

    links {"SQLite"}

    debugdir "."

    filter "configurations:Debug"
        defines "_DEBUG"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        defines "_RELEASE"
        runtime "Release"
        optimize "on"

    filter "system:windows"
        defines { "PLATFORM_WINDOWS" }
    
    filter "system:linux"
        defines { "PLATFORM_LINUX" }



project "SQLite"
    location "Dependencies/SQLite"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    systemversion "latest"

    files {
        "Dependencies/%{prj.name}/sqlite-amalgamation-3460100/**.h",
        "Dependencies/%{prj.name}/sqlite-amalgamation-3460100/**.c"
    }

    filter "configurations:Debug"
        defines "_DEBUG"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        defines "_RELEASE"
        runtime "Release"
        optimize "on"