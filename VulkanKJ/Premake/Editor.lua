include "common.lua"

project ("Editor")
	location (dirs.temp)
	
	-- pchheader ("enginepch.h")
    -- pchsource ("Engine/enginepch.cpp")

	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir (dirs.temp)
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.temp}/%{prj.name}/%{cfg.buildcfg}")
	dependson { "Engine" }

includedirs {
		dirs.engine,
		dirs.editor,
		dirs.external,
		dirs.external .. "imgui/",
		dirs.external .. "DirectXTex/",
		dirs.external .. "ffmpeg-2.0/",
		dirs.external .. "spdlog/include/",
		dirs.external .. "magic_enum/include/",
		dirs.external .. "recastnavigation/DebugUtils/include",
		dirs.external .. "recastnavigation/Detour/include",
		dirs.external .. "recastnavigation/DetourCrowd/include",
		dirs.external .. "recastnavigation/DetourTileCache/include",
		dirs.external .. "recastnavigation/Recast/include",
	}

	links {
		"Engine",
		"glfw3.lib",
		"vulkan-1.lib",
	}

	libdirs { dirs.lib }
	files {
		dirs.editor .. "**.h",
		dirs.editor .. "**.hpp",
		dirs.editor .. "**.cpp",
		dirs.editor .. "**.natvis",
	}

	enableunitybuild "On"
