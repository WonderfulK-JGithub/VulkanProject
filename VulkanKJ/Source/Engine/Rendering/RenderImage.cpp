#include "RenderImage.h"

#include "Engine.h"
#include "imgui_impl_vulkan.h"
#include "Vulkan/Image/Image.h"
#include "Util/Logging/Logging.h"
#include "Vulkan/Image/Sampler.h"

namespace 
{
    RenderImageId localIdCounter = 0;
}

void RenderImage::Init(uint32_t aWidth, uint32_t aHeight,vk::Format aImageFormat, bool aIsDepthImage)
{
    Engine& engine = Engine::GetInstance();
    Vulkan::ImageInput imageInput;
    imageInput.logicalDevice    = engine.GetLogicalDevice();
    imageInput.physicalDevice   = engine.GetPhysicalDevice();
    imageInput.width            = aWidth;
    imageInput.height           = aHeight;
    imageInput.format           = aImageFormat;
    imageInput.arrayCount       = 1;
    imageInput.tiling           = vk::ImageTiling::eOptimal;
    imageInput.usage            = (aIsDepthImage ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment) 
    | vk::ImageUsageFlagBits::eSampled;
    imageInput.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    imageInput.flags            = vk::ImageCreateFlags();
    
    for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); i++)
    {
        myImages.push_back(MakeImage(imageInput));
        myImageMemories.push_back(MakeImageMemory(imageInput, myImages.back()));
        myImageViews.push_back(Vulkan::MakeImageView(engine.GetLogicalDevice(), myImages.back(), aImageFormat, 
            aIsDepthImage ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor, 
        vk::ImageViewType::e2D, 1));
        
        myIds.push_back(++localIdCounter);
        Vulkan::TextureDescriptor::GetInstance().AddRenderImage(myIds.back(),myImageViews.back());
    }
    
    myImageLayouts.resize(engine.GetSwapchainFrames().size(),vk::ImageLayout::eUndefined);
    
    myIsDepthImage = aIsDepthImage;
    myIsInitialized = true;
}

void RenderImage::InitFromSwapchainImages(const std::vector<vk::Image>& aSwapchainImages)
{
    Engine& engine = Engine::GetInstance();
    for (uint32_t i = 0; i < aSwapchainImages.size(); i++)
    {
        myImages.push_back(aSwapchainImages[i]);
        myImageViews.push_back(Vulkan::MakeImageView(engine.GetLogicalDevice(), myImages.back(), engine.GetSwapchainColorFormat(), vk::ImageAspectFlagBits::eColor, 
        vk::ImageViewType::e2D, 1));
    }
    
    myImageLayouts.resize(engine.GetSwapchainFrames().size(),vk::ImageLayout::ePresentSrcKHR);// this should never be changed i think
    
    myIsSwapchainImage = true;
    myIsInitialized = true;
}

void RenderImage::Destroy()
{
    auto& engine = Engine::GetInstance();
    auto logicalDevice = engine.GetLogicalDevice();
    for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); ++i)
    {
        if (myIsSwapchainImage == false)
        {
            logicalDevice.freeMemory(myImageMemories[i]);
            logicalDevice.destroyImage(myImages[i]);
        }
        
        logicalDevice.destroyImageView(myImageViews[i]);
        
        if (myIds.empty() == false)
        {
            Vulkan::TextureDescriptor::GetInstance().RemoveRenderImage(myIds[i]);
        }
    }

    for (auto& imGuiDS : myImGuiDescriptorSets)
    {
        ImGui_ImplVulkan_RemoveTexture(imGuiDS);
    }
    
    myImages.clear();
    myImageMemories.clear();
    myImageViews.clear();
    myIds.clear();
    myImageLayouts.clear();
    
    myIsInitialized = false;
}

void RenderImage::TransitionToTarget(vk::CommandBuffer aCommandBuffer)
{
    vk::ImageLayout targetImageLayout = myIsDepthImage ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;
    uint32_t currentFrameIndex = Engine::GetInstance().GetCurrentSwapchainIndex();
    
    if (myImageLayouts[currentFrameIndex] != targetImageLayout)
    {
        Vulkan::TransitionImageLayout(aCommandBuffer,myImages[currentFrameIndex],
            myImageLayouts[currentFrameIndex],targetImageLayout,myIsDepthImage ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor,1);
    }
    else
    {
        //LOG_WARNING("%s","Render image was already transitioned to target!");
    }
    
    myImageLayouts[currentFrameIndex] = targetImageLayout;
}

void RenderImage::TransitionToResource(vk::CommandBuffer aCommandBuffer)
{
    uint32_t currentFrameIndex = Engine::GetInstance().GetCurrentSwapchainIndex();
    
    if (myImageLayouts[currentFrameIndex] != vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        Vulkan::TransitionImageLayout(aCommandBuffer,myImages[currentFrameIndex],
            myImageLayouts[currentFrameIndex],vk::ImageLayout::eShaderReadOnlyOptimal,myIsDepthImage ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor, 1);
    }
    else
    {
        //LOG_WARNING("%s","Render image was already transitioned to resource!");
    }
    
    myImageLayouts[currentFrameIndex] = vk::ImageLayout::eShaderReadOnlyOptimal;
}

void RenderImage::CreateImGuiDescriptorSets()
{
    auto& engine = Engine::GetInstance();
    myImGuiDescriptorSets.resize(engine.GetSwapchainFrames().size());
    for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); ++i)
    {
        myImGuiDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(
                    Vulkan::GetSampler(Vulkan::SamplerType::BilinearRepeat),
                    myImageViews[i],
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                );
    }
}

VkDescriptorSet RenderImage::GetImGuiDescriptorSet(const uint32_t aFrameNumber) const
{
    return myImGuiDescriptorSets[aFrameNumber];
}

uint32_t RenderImage::GetRenderImageId(uint32_t aFrameNumber) const
{
    return myIds[aFrameNumber];
}

vk::ImageView RenderImage::GetImageView(const uint32_t aFrameNumber) const
{
    return myImageViews[aFrameNumber];
}

bool RenderImage::IsDepthImage() const
{
    return myIsDepthImage;
}

bool RenderImage::IsInitialized() const
{
    return myIsInitialized;
}

bool RenderImage::IsSwapchainImage() const
{
    return myIsSwapchainImage;
}
