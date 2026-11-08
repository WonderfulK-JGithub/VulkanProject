#pragma once
#include <vulkan/vulkan.hpp>

#include "Id/IdTypes.h"
#include "Rendering/RenderImage.h"
#include "Util/Container/MappedVector.hpp"

namespace Vulkan
{
    constexpr int MAX_TEXTURES = 128;
    constexpr int MAX_CUBE_MAPS = 16;
    
    class TextureDescriptor
    {
    public:
        static TextureDescriptor& GetInstance();
        
        void Init();
        void Destroy();
        vk::DescriptorSet GetDescriptorSet();
        vk::DescriptorSetLayout GetDescriptorSetLayout();
        
        void AddTexture(Wonder::TextureId aId,vk::ImageView aView,bool aIsCubemap);
        void RemoveTexture(Wonder::TextureId aId,bool aIsCubemap);
        uint32_t GetTextureIndex(Wonder::TextureId aId);
        uint32_t GetCubemapIndex(Wonder::TextureId aId);
        
        void AddRenderImage(RenderImageId aId,vk::ImageView aView);
        void RemoveRenderImage(RenderImageId aId);
        uint32_t GetRenderImageTextureIndex(RenderImageId aId);
        
        bool NeedsDescriptorUpdate() const;
        void UpdateDescriptorSet();
    private:
        vk::DescriptorSetLayout myDescriptorSetLayout;
        vk::DescriptorSet myDescriptorSet;
        
        Goose::MappedVector<Wonder::TextureId,vk::ImageView> myIdToView;
        std::unordered_map<Wonder::TextureId,uint32_t> myIdToIndex;
        
        Goose::MappedVector<RenderImageId,vk::ImageView> myRenderImageIdToView;
        std::unordered_map<RenderImageId,uint32_t> myRenderImageIdToIndex;
        
        Goose::MappedVector<Wonder::TextureId,vk::ImageView> myCubemapIdToView;
        std::unordered_map<Wonder::TextureId,uint32_t> myCubemapIdToIndex;
        
        bool myNeedsDescriptorUpdate{};
    };
}


