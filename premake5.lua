workspace "PML"
   configurations { "Debug", "Release" }
   platforms { "Win64", "Linux" }

   filter { "platforms:Win64" }
      system "Windows"
      architecture "x64"

project "PML"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"

   files { "src/**.h", "src/**.cpp" }

   filter { "platforms:Linux" }
      defines { "PLATFORM_LINUX" }
      links { "LLVM-7" }

   filter { "platforms:Win64" }
      defines { "PLATFORM_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"