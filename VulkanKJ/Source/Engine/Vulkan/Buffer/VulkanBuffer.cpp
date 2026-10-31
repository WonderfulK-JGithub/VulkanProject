#include "VulkanBuffer.h"

#include "Engine.h"

void Vulkan::VulkanBuffer::Init(const size_t aSize, const vk::BufferUsageFlagBits aBufferType)
{
    Engine& engine = Engine::GetInstance();

    for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); ++i)
    {
        Vulkan::BufferCreationInput input;
        input.logicalDevice = engine.GetLogicalDevice();
        input.physicalDevice = engine.GetPhysicalDevice();
        input.memortyProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        input.size = aSize;
        input.usage = aBufferType;
        
        myBuffers.push_back(Vulkan::CreateBuffer(input));
        myWriteLocations.push_back(engine.GetLogicalDevice().mapMemory(myBuffers.back().bufferMemory, 0, aSize));
        
        vk::DescriptorBufferInfo info{};
        info.buffer = myBuffers.back().buffer;
        info.offset = 0;
        info.range = aSize;
        myDescriptorBufferInfos.push_back(info);
    }
}

void Vulkan::VulkanBuffer::Destroy()
{
    Engine& engine = Engine::GetInstance();
    vk::Device logicalDevice = engine.GetLogicalDevice();
    for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); ++i)
    {
        logicalDevice.unmapMemory(myBuffers[i].bufferMemory);
        logicalDevice.freeMemory(myBuffers[i].bufferMemory);
        logicalDevice.destroyBuffer(myBuffers[i].buffer);
    }
    
    myBuffers.clear();
    myWriteLocations.clear();
    myDescriptorBufferInfos.clear();
}

void* Vulkan::VulkanBuffer::GetWriteLocation() const
{
    return myWriteLocations[Engine::GetInstance().GetCurrentSwapchainIndex()];
}

const Vulkan::Buffer& Vulkan::VulkanBuffer::GetBuffer() const
{
    return myBuffers[Engine::GetInstance().GetCurrentSwapchainIndex()];
}

vk::DescriptorBufferInfo Vulkan::VulkanBuffer::GetDescriptorBufferInfo() const
{
    return myDescriptorBufferInfos[Engine::GetInstance().GetCurrentSwapchainIndex()];
}

const std::vector<vk::DescriptorBufferInfo>& Vulkan::VulkanBuffer::GetAllDescriptorBufferInfos() const
{
    return myDescriptorBufferInfos;
}
