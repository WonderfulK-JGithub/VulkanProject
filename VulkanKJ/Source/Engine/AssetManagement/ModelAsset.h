#pragma once
#include <nlohmann/json.hpp>

#include "Vulkan/VkMesh.h"
#include "ID/IdTypes.h"

namespace Wonder
{
    class ModelAsset
    {
    public:
        void Init(const nlohmann::json& aJson);
        void SaveToJson(nlohmann::json& aJson);
    
        void InitEngine(Wonder::ModelId aId);
    
        void Load();
        void LoadCustom(std::vector<Vulkan::PosColorVertex>& aVertices, std::vector<uint32_t>& aIndices);
        
        void Unload();
        bool IsLoaded() const;
    
        const std::vector<Vulkan::MeshData>& GetMeshes() const;
    private:
        std::vector<Vulkan::MeshData> myMeshes;
    
        Wonder::ModelId myId;
        bool myIsEngineAsset{};
        bool myIsLoaded{};
    };
}


