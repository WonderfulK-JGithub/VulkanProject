include "common.lua"
include "extensions.lua"

workspace "Wonderful_Engine"
	location "../"
	startproject "Launcher"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Retail"
	}

	warnings "extra"
	fatalwarnings {
		"All"
	}

	filter "configurations:Debug"
		defines {
			"_DEBUG", 
			"DEBUG",
		}
		runtime "Debug"
		symbols "on"
		optimize "Off"

	filter "configurations:Release"
		defines {
			"_RELEASE", 
			"NDEBUG", 
		}
		runtime "Release"
		symbols "on"
		optimize "Debug"

	filter "configurations:Retail"
		defines {
			"_RETAIL", 
			"NDEBUG",
		}
		runtime "Release"
		symbols "off"
		optimize "Full"

	conformancemode "Yes"

-- include for common stuff 
-- include "common.lua"

include "Editor.lua"

-------------------------------------------------------------
include "Engine.lua"

-----------------------

project ("Launcher")
	location (dirs.temp)
		
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	enableunitybuild "On"
	debugdir "%{dirs.bin}"
	targetdir ("%{dirs.bin}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.temp}/%{prj.name}/%{cfg.buildcfg}")

	includedirs { dirs.engine,dirs.external }
	libdirs {dirs.lib }
	-- libdirs { dirs.dependencies,dirs.bin }
	dependson { "Engine","Editor" }
	links {
		"Engine",
		"Editor",
		-- "External",
		"glfw3.lib",
		"vulkan-1.lib",
	}

	files {
		dirs.editor_launcher .. "**.h",
		dirs.editor_launcher .. "**.hpp",
		dirs.editor_launcher .. "**.cpp",
		dirs.editor_launcher .. "**.natvis",
	}

	-- shadermodel("5.0")

	-- filter("files:**.hlsl")
	-- 	flags("ExcludeFromBuild")
	-- 	shaderobjectfileoutput(dirs.shader_dir.."%{file.basename}"..".cso")

	-- filter("files:**PS.hlsl")
	-- 	removeflags("ExcludeFromBuild")
	-- 	shadertype("Pixel")

	-- filter("files:**VS.hlsl")
	-- 	removeflags("ExcludeFromBuild")
	-- 	shadertype("Vertex")

	-- filter("files:**GS.hlsl")
	-- 	removeflags("ExcludeFromBuild")
	-- 	shadertype("Geometry")

verify_or_create_settings("Wonderful_Engine")