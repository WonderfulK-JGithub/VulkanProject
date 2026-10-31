#pragma once
#include "Vulkan/VkMemory.h"

namespace Vulkan
{
    class VulkanBuffer
    {
    public:
        void Init(size_t aSize,vk::BufferUsageFlagBits aBufferType);
        void Destroy();
        
        void* GetWriteLocation() const;
        const Buffer& GetBuffer() const;
        vk::DescriptorBufferInfo GetDescriptorBufferInfo() const;
        
        const std::vector<vk::DescriptorBufferInfo>& GetAllDescriptorBufferInfos() const;
    private:
        
        std::vector<Vulkan::Buffer> myBuffers;
        std::vector<void*> myWriteLocations;
        std::vector<vk::DescriptorBufferInfo> myDescriptorBufferInfos;
        
    };
}
