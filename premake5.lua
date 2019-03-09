workspace "PML"
   configurations { "Debug", "Release" }
   platforms { "Win64" }

   filter { "platforms:Win64" }
      system "Windows"
      architecture "x64"

project "PML"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"

   files { "src/**.h", "src/**.cpp" }

   filter "configurations:Debug"
      defines { "DEBUG", "_CRT_SECURE_NO_WARNINGS" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG", "_CRT_SECURE_NO_WARNINGS" }
      optimize "On"