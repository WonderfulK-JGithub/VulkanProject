include "common.lua"

project ("Engine")
	location (dirs.temp)
	
	-- pchheader ("enginepch.h")
    -- pchsource ("Engine/enginepch.cpp")

	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir (dirs.temp)
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.temp}/%{prj.name}/%{cfg.buildcfg}")

includedirs {
		dirs.engine,
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
		dirs.external .. "spdlog/include/",
	}

	links {
		-- "External",
		"glfw3.lib",
		"vulkan-1.lib",
		"shaderc_shared",
	}

	libdirs { dirs.lib }
	files {
		dirs.engine .. "**.h",
		dirs.engine .. "**.hpp",
		dirs.engine .. "**.cpp",
		dirs.engine .. "**.natvis",
		dirs.engine .. "**.vert",
		dirs.engine .. "**.frag",

		dirs.external .. "imgui/imgui.cpp",
		dirs.external .. "imgui/imgui_demo.cpp",
		dirs.external .. "imgui/imgui_draw.cpp",
		dirs.external .. "imgui/imgui_tables.cpp",
		dirs.external .. "imgui/imgui_widgets.cpp",
		dirs.external .. "imgui/imgui.h",

		dirs.external .. "imgui/imgui_impl_glfw.cpp",
		dirs.external .. "imgui/imgui_impl_glfw.h",
		dirs.external .. "imgui/imgui_impl_vulkan.cpp",
		dirs.external .. "imgui/imgui_impl_vulkan.h",
	}

	enableunitybuild "On"
	buildoptions { "/bigobj" }
