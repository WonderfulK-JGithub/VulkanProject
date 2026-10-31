#include "VkCommands.h"
#include <iostream>

#include "Util/Logging/Logging.h"

vk::CommandPool VkInit::MakeCommandPool(vk::Device aDevice, vk::PhysicalDevice aPhysicalDevice, vk::SurfaceKHR aSurface)
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(aPhysicalDevice, aSurface);

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    try
    {
        return aDevice.createCommandPool(poolInfo);
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create command pool, %s",err.what());
    }

    return {};
}

vk::CommandBuffer VkInit::MakeCommandBuffer(vk::Device aDevice, vk::CommandPool aCommandPool)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = aCommandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    try
    {
        return aDevice.allocateCommandBuffers(allocInfo)[0];
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create main command buffer, %s",err.what());
    }

    return {};
}

void VkInit::MakeFrameCommandBuffers(const CommandBufferInput& aInput)
{

    for (int i = 0; i < aInput.frames.size(); i++)
    {
        aInput.frames[i].commandBuffer = MakeCommandBuffer(aInput.device, aInput.commandPool);

    }
}
