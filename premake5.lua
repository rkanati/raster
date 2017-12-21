workspace "raster"
  configurations { "debug", "release" }

project "raster"
  kind "ConsoleApp"
  language "C++"
  targetdir "%{cfg.buildcfg}/bin"

  files { "src/**.hpp", "src/**.cpp" }

  -- includedirs { "rk-core/include", "rk-math/include" }
  -- links { "pthread", "pulse" }

  warnings "Extra"

  filter "configurations:debug"
    defines { "DEBUG" }
    symbols "On"
    warnings "Extra"

  filter "configurations:release"
    defines { "NDEBUG" }
    optimize "On"

  filter "toolset:gcc"
    buildoptions { "-std=c++14" } --, "-pthread" }

