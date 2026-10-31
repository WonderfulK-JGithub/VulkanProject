#include "SingleTimeCommands.h"

void Vulkan::StartSingleTimeCommand(vk::CommandBuffer aCommandBuffer)
{
    aCommandBuffer.reset();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    aCommandBuffer.begin(beginInfo);
}

void Vulkan::EndSingleTimeCommand(vk::CommandBuffer aCommandBuffer, vk::Queue aQueue)
{
    aCommandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &aCommandBuffer;
    [[maybe_unused]] vk::Result submitResult = aQueue.submit(1, &submitInfo, {});
    aQueue.waitIdle();
}
