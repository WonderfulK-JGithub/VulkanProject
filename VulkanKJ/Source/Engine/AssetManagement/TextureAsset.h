#pragma once
#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>

#include "ID/IdTypes.h"
#include "Vulkan/Image/Texture.h"

namespace Wonder
{
    class TextureAsset
    {
    public:
        void Init(const nlohmann::json& aJson);
        void SaveToJson(nlohmann::json& aJson);
        
        void InitEngine(TextureId aId);
        
        void Load();
        void Unload();
        bool IsLoaded() const;
        
        VkDescriptorSet& GetImGuiDescriptorSet();
    private:
        Vulkan::Texture myTexture;
        VkDescriptorSet myImGuiDescriptorSet{};
        
        TextureId myId;
        bool myIsEngineAsset{};
        bool myIsLoaded{};
    };
}


