#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>

using RenderImageId = uint32_t;

class RenderImage
{
public:
    void Init(uint32_t aWidth, uint32_t aHeight,vk::Format aImageFormat, bool aIsDepthImage);
    void InitFromSwapchainImages(const std::vector<vk::Image>& aSwapchainImages);
    
    void Destroy();
    
    void TransitionToTarget(vk::CommandBuffer aCommandBuffer);
    void TransitionToResource(vk::CommandBuffer aCommandBuffer);
    
    void CreateImGuiDescriptorSets();
    
    vk::ImageView GetImageView(uint32_t aFrameNumber) const;
    VkDescriptorSet GetImGuiDescriptorSet(uint32_t aFrameNumber) const;
    uint32_t GetRenderImageId(uint32_t aFrameNumber) const;
    
    bool IsDepthImage() const;
    bool IsInitialized() const;
    bool IsSwapchainImage() const;
private:
    std::vector<vk::Image> myImages;
    std::vector<vk::DeviceMemory> myImageMemories;
    std::vector<vk::ImageView> myImageViews;
    
    std::vector<VkDescriptorSet> myImGuiDescriptorSets{};
    
    std::vector<vk::ImageLayout> myImageLayouts;
    
    std::vector<RenderImageId> myIds;
    
    bool myIsDepthImage{};
    bool myIsSwapchainImage{};
    bool myIsInitialized{};
};
