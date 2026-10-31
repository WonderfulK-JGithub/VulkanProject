#pragma once
#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>

#include "ECS/Reflection/ComponentData.h"
#include "ID/IdTypes.h"
#include "Vulkan/Image/Texture.h"

namespace Wonder
{
    class PrefabAsset
    {
    public:
        void Init(const nlohmann::json& aJson);
        void SaveToJson(nlohmann::json& aJson);
        
        std::vector<ComponentData>& AccessComponents();
    private:
        std::vector<ComponentData> myComponents;
        
        PrefabId myId;
    };
}


