#include "VkMemory.h"
#include "SingleTimeCommands.h"

Vulkan::Buffer Vulkan::CreateBuffer(const BufferCreationInput& aInput)
{
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.flags = vk::BufferCreateFlags();
    bufferInfo.size = aInput.size;
    bufferInfo.usage = aInput.usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    Buffer buffer{};
    buffer.buffer = aInput.logicalDevice.createBuffer(bufferInfo);
    AllocateBufferMemory(buffer, aInput);

    return buffer;
}

uint32_t Vulkan::FindMemoryTypeIndex(vk::PhysicalDevice aPhysicalDevice, uint32_t aSupportedMemoryIndices, vk::MemoryPropertyFlags aRequestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = aPhysicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        bool isSupported = (aSupportedMemoryIndices & (1 << i)) != 0u;
        bool isSufficient = (memoryProperties.memoryTypes[i].propertyFlags & aRequestedProperties) == aRequestedProperties;

        if (isSupported && isSufficient)
        {
            return i;
        }
    }

    return 0;
}

void Vulkan::AllocateBufferMemory(Buffer& aBuffer, const BufferCreationInput& aInput)
{
    vk::MemoryRequirements memoryRequirements = aInput.logicalDevice.getBufferMemoryRequirements(aBuffer.buffer);

    vk::MemoryAllocateInfo allocateInfo{};
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(aInput.physicalDevice, memoryRequirements.memoryTypeBits,
        aInput.memortyProperties);

    aBuffer.bufferMemory = aInput.logicalDevice.allocateMemory(allocateInfo);
    aInput.logicalDevice.bindBufferMemory(aBuffer.buffer, aBuffer.bufferMemory, 0);
}

void Vulkan::CopyBuffer(Buffer& aSource, Buffer& aDestination, vk::DeviceSize aSize, vk::Queue aQueue, vk::CommandBuffer aCommandBuffer)
{
    StartSingleTimeCommand(aCommandBuffer);

    vk::BufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = aSize;
    aCommandBuffer.copyBuffer(aSource.buffer, aDestination.buffer, 1, &copyRegion);

    EndSingleTimeCommand(aCommandBuffer, aQueue);
}
