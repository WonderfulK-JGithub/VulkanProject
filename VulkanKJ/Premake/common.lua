----------------------------------------------------------------------------
-- the dirs table is a listing of absolute paths, since we generate projects
-- and files it makes a lot of sense to make them absolute to avoid problems
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
dirs = {}
dirs["root"] 			= os.realpath("../")
dirs["bin"]				= os.realpath(dirs.root .. "Bin/")
dirs["temp"]			= os.realpath(dirs.root .. "Temp/")
dirs["source"] 			= os.realpath(dirs.root .. "Source/")
dirs["external"]		= os.realpath(dirs.root .. "External/")
dirs["lib"]		        = os.realpath(dirs.root .. "External/lib")
dirs["engine"]			= os.realpath(dirs.root .. "Source/Engine/")
dirs["engine_tests"]	= os.realpath(dirs.root .. "Source/EngineTests/")
dirs["settings"]		= os.realpath(dirs.root .. "Bin/settings/")
dirs["engine_assets"] 	= os.realpath(dirs.root .. "Assets/Engine/")
dirs["game"]			= os.realpath(dirs.root .. "Source/Game/")
dirs["gamemain"]		= os.realpath(dirs.root .. "Source/GameMain/")
dirs["editor_launcher"]			= os.realpath(dirs.root .. "Source/EditorLauncher/")
dirs["editor"]	    = os.realpath(dirs.root .. "Source/Editor/")
dirs["goose"] 			= os.realpath(dirs.root .. "Source/Goose/")
dirs["cooked_assets"]	= os.realpath(dirs.root .. "Bin/CookedAssets/")
dirs["baked_assets"]	= os.realpath(dirs.root .. "Bin/BakedAssets/")
dirs["assets"]	= os.realpath(dirs.root .. "Assets/")
dirs["data"]	= os.realpath(dirs.root .. "Assets/data")
dirs["levelData"]	= os.realpath(dirs.root .. "Assets/levelData")
dirs["intermediateAssets"]	= os.realpath(dirs.root .. "Assets/intermediateAssets")
dirs["cutsceneData"]	= os.realpath(dirs.levelData .. "/cutsceneData")
dirs["scripts"]	= os.realpath(dirs.root .. "Assets/scripts")
dirs["issues"]= os.realpath(dirs.root .. "Assets/issues")
dirs["noise"] = os.realpath(dirs.source .. "NoiseGenerator/")
dirs["soundBanks"] = os.realpath(dirs.root .. "Assets/soundbanks/")


dirs["shader_dir"] 		= os.realpath(dirs.root .. "Bin/CookedAssets/Shaders/")


engine_settings = os.realpath(dirs.settings .. "/EngineSettings.json")


-----------------------------------------------------------------------
-- These should be more or less equivalent with Engines WindowConfiguration struct
-- Some of it can't be set like this, things like callbacks.
function default_settings()
	return {
		assets_path = {
			bin = dirs.bin;
			engine_assets = path.getrelative(dirs.bin, dirs.engine_assets) .. "/",
			data = path.getrelative(dirs.bin, dirs.data) .. "/",
			cooked = path.getrelative(dirs.bin, dirs.cooked_assets) .. "/",
			baked = path.getrelative(dirs.bin, dirs.baked_assets) .. "/",
			levelData = path.getrelative(dirs.bin, dirs.cooked_assets) .. "/",
			cutsceneData = path.getrelative(dirs.bin, dirs.cutsceneData) .. "/",
			assets = path.getrelative(dirs.bin, dirs.assets) .. "/",
			source = path.getrelative(dirs.bin, dirs.source) .. "/",
			intermediateAssets = path.getrelative(dirs.bin, dirs.intermediateAssets) .. "/"
		},

		window_settings = {
			window_size = {w=1600, h=900},
			render_size = {w=1600, h=900},
			target_size = {w=1600, h=900},
			-- title = "No celeste no life",
			title = "Wonderful Engine",
	 		clear_color = {r=0.0, g=0.2, b=0.25, a=1.0},

			keep_aspect_ratio = true,
			aspect_ratio = 1.7,

	 		start_in_fullscreen = false
		},
		enable_vsync = false,
	}
end

if not os.isdir (dirs.bin) then
	os.mkdir (dirs.bin)
end

if not os.isdir (dirs.cooked_assets) then
	os.mkdir (dirs.cooked_assets)
end

if not os.isdir (dirs.baked_assets) then
	os.mkdir (dirs.baked_assets)
end

if not os.isdir (path.getrelative(dirs.bin, dirs.levelData)) then
	os.mkdir (path.getrelative(dirs.bin, dirs.levelData))
end

if not os.isdir (path.getrelative(dirs.bin, dirs.data)) then
	os.mkdir (path.getrelative(dirs.bin, dirs.data))
end

if not os.isdir (path.getrelative(dirs.bin, dirs.intermediateAssets)) then
	os.mkdir (path.getrelative(dirs.bin, dirs.intermediateAssets))
	os.mkdir (path.getrelative(dirs.bin, dirs.intermediateAssets .. "/Textures/"))
	os.mkdir (path.getrelative(dirs.bin, dirs.intermediateAssets) .. "/Models/")
end

if not os.isdir (path.getrelative(dirs.bin, dirs.cutsceneData)) then
	os.mkdir (path.getrelative(dirs.bin, dirs.cutsceneData))
end

if not os.isdir(dirs.settings) then 
	os.mkdir (dirs.settings)
end

if not os.isdir(dirs.scripts) then 
	os.mkdir (dirs.scripts)
end

if not os.isdir(dirs.issues) then 
	os.mkdir (dirs.issues)
end

---------------------------------------------------------------------------
-- Utility function to create individual project-settings any game project, 
-- tutorial or similar is responsible for running:
-- Tga::LoadSettings("game_name.json"); at startup
function verify_or_create_settings(game_name)
	local settings_filename = game_name .. ".json"
	defines { 'TGE_PROJECT_SETTINGS_FILE="' .. settings_filename .. '"' }
	local game_settings = dirs["settings"] .. settings_filename
	
	local settings = default_settings()
	-- if os.isfile(game_settings) then
	-- 	local old_settings = json.decode(io.readfile(game_settings))
	-- 	for k,v in pairs(old_settings) do
	-- 		settings[k] = v
	-- 	end

	-- 	settings.assets_path.engine_assets = path.getrelative(dirs.bin, dirs.engine_assets) .. "/"
	-- 	settings.assets_path.data = path.getrelative(dirs.bin, dirs.data) .. "/"
	-- 	settings.assets_path.levelData = path.getrelative(dirs.bin, dirs.levelData) .. "/"
	-- 	settings.assets_path.intermediateAssets = path.getrelative(dirs.bin, dirs.intermediateAssets) .. "/"
	-- 	settings.assets_path.cutsceneData = path.getrelative(dirs.bin, dirs.cutsceneData) .. "/"
	-- 	settings.assets_path.scripts = path.getrelative(dirs.bin, dirs.scripts) .. "/"
	-- 	settings.assets_path.issues = path.getrelative(dirs.bin, dirs.issues) .. "/"
	-- 	settings.assets_path.root = path.getrelative(dirs.bin, dirs.root) .. "/"
	-- 	settings.assets_path.soundBanks = path.getrelative(dirs.bin, dirs.soundBanks) .. "/"
	-- 	settings.assets_path.source = path.getrelative(dirs.bin, dirs.source) .. "/"
	-- 	settings.assets_path.assets = path.getrelative(dirs.bin, dirs.assets) .. "/"
	-- 	-- settings.assets_path.engine = path.translate(dirs.engine_assets, "/")
	-- 	-- settings.assets_path.game = path.translate(os.realpath("./data/"), "/")
	-- end

	io.writefile(
		game_settings,
		json.encode(settings)
	)
end