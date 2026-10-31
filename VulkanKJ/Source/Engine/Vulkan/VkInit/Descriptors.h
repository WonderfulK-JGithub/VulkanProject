#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
#include "../RenderStructs.h"

namespace Vulkan
{
	class VulkanBuffer;
}

namespace VkInit
{
	
	void CreateStageDescriptorSetLayouts();
	void DestroyStageDescriptorSetLayouts();

	vk::DescriptorSetLayout GetStageDescriptorSetLayout(Vulkan::RenderStageType aStage);
	
	struct DescriptorSetLayoutData
	{
		vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlags();
		int count;
		std::vector<int> indices;
		std::vector<vk::DescriptorType> types;
		std::vector<int> counts;
		std::vector<vk::ShaderStageFlags> stages;
	};

	vk::DescriptorSetLayout MakeDescriptorSetLayout(vk::Device aDevice, const DescriptorSetLayoutData& aInput);

	vk::DescriptorPool MakeDescriptorPool(vk::Device aDevice, uint32_t aSize, const DescriptorSetLayoutData& aInput);

	vk::DescriptorSet AllocateDescriptorSet(vk::Device aDevice, vk::DescriptorPool aDescriptorPool, vk::DescriptorSetLayout aLayout);
	
	class VulkanDescriptorSet
	{
	public:
		void Init(vk::DescriptorPool aDescriptorPool, vk::DescriptorSetLayout aLayout);
		
		vk::DescriptorSet GetDescriptorSet() const;
		
		const std::vector<vk::DescriptorSet>& GetAllDescriptorSets() const;
	private:
		std::vector<vk::DescriptorSet> myDescriptorSets;
	};
	
	class DescriptorWriter
	{
	public:
		void AddDescriptorInfo(const VulkanDescriptorSet& aDescriptorSet, const Vulkan::VulkanBuffer& aBuffer,vk::DescriptorType aType,uint32_t aBinding);
		
		void UpdateDescriptorSets() const;
		
		void Reset();
	private:
		std::vector<std::vector<vk::WriteDescriptorSet>> myWriteOperations;
	};
}
