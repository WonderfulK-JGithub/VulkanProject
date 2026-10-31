#pragma once
#include <filesystem>

using Filepath = std::filesystem::path;

namespace Wonder
{
    constexpr const char* SCENE_FILE_EXTENSION = ".sce";
    constexpr const char* UI_FILE_EXTENSION = ".uid";
    constexpr const char* LIGHT_FILE_EXTENSION = ".lig";

    constexpr const char* PREFAB_FILE_EXTENSION         = ".pre";
    constexpr const char* MATERIAL_FILE_EXTENSION       = ".mat";
    constexpr const char* PARTICLE_FILE_EXTENSION       = ".par";
    constexpr const char* ANIMCONTROLLER_FILE_EXTENSION = ".anr";
    constexpr const char* TEXTURE_FILE_EXTENSION        = ".tex";
    constexpr const char* MODEL_FILE_EXTENSION          = ".mod";
    constexpr const char* ANIMATION_FILE_EXTENSION      = ".anm";
    constexpr const char* CUTSCENE_FILE_EXTENSION       = ".cut";
    constexpr const char* TERRAIN_FILE_EXTENSION        = ".terrain";


    constexpr const char* NOISE_FILE_EXTENSION			= ".noise";


    constexpr const char* SCRIPT_FILE_EXTENSION = ".script";
}
