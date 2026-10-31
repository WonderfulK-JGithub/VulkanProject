#include "VkMesh.h"

#include <fstream>
#include "Util/Logging/Logging.h"

vk::PipelineVertexInputStateCreateInfo MakeVertexInputInfo(const vk::VertexInputBindingDescription& aBindingDescription,
    const std::vector<vk::VertexInputAttributeDescription>& aAttributeDescription)
{
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.flags                           = vk::PipelineVertexInputStateCreateFlags();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(aAttributeDescription.size());
    vertexInputInfo.pVertexAttributeDescriptions    = aAttributeDescription.data();
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.pVertexBindingDescriptions      = &aBindingDescription;

    return vertexInputInfo;
}

namespace Vulkan
{
    struct VertexInfo
    {
        vk::PipelineVertexInputStateCreateInfo createInfo;
        vk::VertexInputBindingDescription bindDescription;
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    };

    std::unordered_map<VertexLayoutType, VertexInfo> vertexInfos;
    
    static Vulkan::MeshPool* localInstance;
}



void Vulkan::CreateVertexInputInfos()
{
    VertexInfo& defaultVertexInfo = vertexInfos[VertexLayoutType::PosColor];
    defaultVertexInfo.bindDescription = GetPosColorBindingDescription();
    defaultVertexInfo.attributeDescriptions = GetPosColorAttributeDescriptions();
    defaultVertexInfo.createInfo = MakeVertexInputInfo(defaultVertexInfo.bindDescription, defaultVertexInfo.attributeDescriptions);
    
    VertexInfo& lineVertexInfo = vertexInfos[VertexLayoutType::Line];
    lineVertexInfo.bindDescription = GetLineBindingDescription();
    lineVertexInfo.attributeDescriptions = GetLinerAttributeDescriptions();
    lineVertexInfo.createInfo = MakeVertexInputInfo(lineVertexInfo.bindDescription, lineVertexInfo.attributeDescriptions);

    VertexInfo& emptyVertexInfo = vertexInfos[VertexLayoutType::Empty];
    emptyVertexInfo.createInfo.flags = vk::PipelineVertexInputStateCreateFlags();
    emptyVertexInfo.createInfo.vertexAttributeDescriptionCount = 0;
    emptyVertexInfo.createInfo.pVertexAttributeDescriptions = nullptr;
    emptyVertexInfo.createInfo.vertexBindingDescriptionCount = 0;
    emptyVertexInfo.createInfo.pVertexBindingDescriptions = nullptr;
}

vk::PipelineVertexInputStateCreateInfo Vulkan::GetVertexInputInfo(VertexLayoutType aType)
{
    const VertexInfo& info = vertexInfos.at(aType);
    return info.createInfo;
}

vk::VertexInputBindingDescription Vulkan::GetPosColorBindingDescription()
{
    vk::VertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(PosColorVertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;

    return bindingDescription;
}

std::vector<vk::VertexInputAttributeDescription> Vulkan::GetPosColorAttributeDescriptions()
{
    std::vector<vk::VertexInputAttributeDescription> attributes;
    attributes.resize(4);


    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = vk::Format::eR32G32B32Sfloat;
    attributes[0].offset = 0;

    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = vk::Format::eR32G32B32Sfloat;
    attributes[1].offset = sizeof(float) * 3;

    attributes[2].binding = 0;
    attributes[2].location = 2;
    attributes[2].format = vk::Format::eR32G32Sfloat;
    attributes[2].offset = sizeof(float) * 6;

    attributes[3].binding = 0;
    attributes[3].location = 3;
    attributes[3].format = vk::Format::eR32G32B32Sfloat;
    attributes[3].offset = sizeof(float) * 8;

    return attributes;
}

vk::VertexInputBindingDescription Vulkan::GetLineBindingDescription()
{
    vk::VertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 1;
    bindingDescription.stride = sizeof(LineVertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;

    return bindingDescription;
}

std::vector<vk::VertexInputAttributeDescription> Vulkan::GetLinerAttributeDescriptions()
{
    std::vector<vk::VertexInputAttributeDescription> attributes;
    attributes.resize(2);

    attributes[0].binding = 1;
    attributes[0].location = 0;
    attributes[0].format = vk::Format::eR32G32B32Sfloat;
    attributes[0].offset = 0;

    attributes[1].binding = 1;
    attributes[1].location = 1;
    attributes[1].format = vk::Format::eR32G32B32A32Sfloat;
    attributes[1].offset = sizeof(float) * 3;
    
    return attributes;
}

Vulkan::MeshPool::MeshPool()
{
    localInstance = this;
}

Vulkan::MeshPool::~MeshPool()
{
    myLogicalDevice.destroyBuffer(myVertexBuffer.buffer);
    myLogicalDevice.freeMemory(myVertexBuffer.bufferMemory);

    myLogicalDevice.destroyBuffer(myIndexBuffer.buffer);
    myLogicalDevice.freeMemory(myIndexBuffer.bufferMemory);
}

Vulkan::MeshPool& Vulkan::MeshPool::GetInstance()
{
    return *localInstance;
}

void Vulkan::MeshPool::AddMesh(MeshData& aMeshData)
{
    myMeshData.insert_or_assign(aMeshData.id,&aMeshData);
    myNeedsUpdate = true;
}

void Vulkan::MeshPool::RemoveMesh(MeshId aId)
{
    myMeshData.erase(aId);
    myNeedsUpdate = true;
}

void Vulkan::MeshPool::Finalize(const FinalizeInput& aInput)
{
    myLogicalDevice = aInput.logicalDevice;
    
    if (myHasBeenInitialized)
    {
        myLogicalDevice.destroyBuffer(myVertexBuffer.buffer);
        myLogicalDevice.freeMemory(myVertexBuffer.bufferMemory);

        myLogicalDevice.destroyBuffer(myIndexBuffer.buffer);
        myLogicalDevice.freeMemory(myIndexBuffer.bufferMemory);
        
        firstIndices.clear();
        indexCounts.clear();
        vertexOffsets.clear();
    }
    
    std::vector<PosColorVertex> vertexLump;
    std::vector<uint32_t> indexLump;
    
    uint32_t vertexOffset{};
    uint32_t indexOffset{};
    
    for (uint32_t i = 0; i < myMeshData.size(); i++)
    {
        MeshData& meshData = *myMeshData.data()[i];
        
        uint32_t vertexCount = static_cast<uint32_t>(meshData.vertices.size());
        uint32_t indexCount  = static_cast<uint32_t>(meshData.indices.size());
        uint32_t lastIndex   = static_cast<uint32_t>(indexLump.size());

        firstIndices.insert(std::make_pair(meshData.id, lastIndex));
        indexCounts.insert(std::make_pair(meshData.id, indexCount));
        vertexOffsets.insert(std::make_pair(meshData.id, vertexOffset));

        vertexLump.resize(vertexLump.size() + meshData.vertices.size());
        std::copy_n(meshData.vertices.data(), meshData.vertices.size(), vertexLump.data() + vertexOffset);
    
        indexLump.resize(indexLump.size() + meshData.indices.size());
        std::copy_n(meshData.indices.data(), meshData.indices.size(), indexLump.data() + indexOffset);

        vertexOffset += vertexCount;
        indexOffset += indexCount;
    }
    
    
    

    {
        Vulkan::BufferCreationInput input;
        input.logicalDevice = aInput.logicalDevice;
        input.physicalDevice = aInput.physicalDevice;
        input.size = sizeof(PosColorVertex) * vertexLump.size();
        input.usage = vk::BufferUsageFlagBits::eTransferSrc;
        input.memortyProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        Vulkan::Buffer intermediateBuffer = Vulkan::CreateBuffer(input);

        void* memoryLocation = aInput.logicalDevice.mapMemory(intermediateBuffer.bufferMemory, 0, input.size);
        memcpy(memoryLocation, vertexLump.data(), input.size);
        aInput.logicalDevice.unmapMemory(intermediateBuffer.bufferMemory);


        input.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
        input.memortyProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

        myVertexBuffer = Vulkan::CreateBuffer(input);

        Vulkan::CopyBuffer(intermediateBuffer, myVertexBuffer, input.size, aInput.queue, aInput.commandBuffer);

        aInput.logicalDevice.destroyBuffer(intermediateBuffer.buffer);
        aInput.logicalDevice.freeMemory(intermediateBuffer.bufferMemory);
    }

    {
        Vulkan::BufferCreationInput input;
        input.logicalDevice = aInput.logicalDevice;
        input.physicalDevice = aInput.physicalDevice;
        input.size = sizeof(uint32_t) * indexLump.size();
        input.usage = vk::BufferUsageFlagBits::eTransferSrc;
        input.memortyProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        Vulkan::Buffer intermediateBuffer = Vulkan::CreateBuffer(input);

        void* memoryLocation = aInput.logicalDevice.mapMemory(intermediateBuffer.bufferMemory, 0, input.size);
        memcpy(memoryLocation, indexLump.data(), input.size);
        aInput.logicalDevice.unmapMemory(intermediateBuffer.bufferMemory);


        input.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
        input.memortyProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

        myIndexBuffer = Vulkan::CreateBuffer(input);

        Vulkan::CopyBuffer(intermediateBuffer, myIndexBuffer, input.size, aInput.queue, aInput.commandBuffer);

        aInput.logicalDevice.destroyBuffer(intermediateBuffer.buffer);
        aInput.logicalDevice.freeMemory(intermediateBuffer.bufferMemory);
    }

    LOG_INFO("Creating a memory pool with:\nVertex count: %d, Index count: %d", vertexLump.size(), indexLump.size());
    
    myHasBeenInitialized = true;
    myNeedsUpdate = false;
}

bool Vulkan::MeshPool::NeedsUpdate() const
{
    return myNeedsUpdate;
}

Vulkan::VertexDrawInfo Vulkan::MeshPool::GetVertexDrawInfo(MeshId aId) const
{
    VertexDrawInfo info{};
    
    info.firstIndex = firstIndices.at(aId);
    info.vertexOffset = vertexOffsets.at(aId);
    info.indexCount = indexCounts.at(aId);
    
    return info;
}
