#include "Descriptors.h"

#include "Engine.h"
#include "Vulkan/Buffer/VulkanBuffer.h"
#include "Util/Logging/Logging.h"

namespace VkInit
{
	static std::unordered_map<Vulkan::RenderStageType, vk::DescriptorSetLayout> descriptorSetLayouts;
}

void VkInit::CreateStageDescriptorSetLayouts()
{
	{
		VkInit::DescriptorSetLayoutData input{};

		input.count = 1;


		input.indices.push_back(0);
		input.types.push_back(vk::DescriptorType::eUniformBuffer);
		input.counts.push_back(1);
		input.stages.push_back(vk::ShaderStageFlagBits::eVertex);

		descriptorSetLayouts[Vulkan::RenderStageType::Cubemap] = VkInit::MakeDescriptorSetLayout(Engine::GetInstance().GetLogicalDevice(), input);
		
		descriptorSetLayouts[Vulkan::RenderStageType::FullscreenSwapchain] = 
			VkInit::MakeDescriptorSetLayout(Engine::GetInstance().GetLogicalDevice(), input);
		descriptorSetLayouts[Vulkan::RenderStageType::FullscreenOffscreen] = 
			VkInit::MakeDescriptorSetLayout(Engine::GetInstance().GetLogicalDevice(), input);
	}

	{
		VkInit::DescriptorSetLayoutData input{};
		input.count = 2;

		input.indices.push_back(0);
		input.types.push_back(vk::DescriptorType::eUniformBuffer);
		input.counts.push_back(1);
		input.stages.push_back(vk::ShaderStageFlagBits::eVertex);

		input.indices.push_back(1);
		input.types.push_back(vk::DescriptorType::eStorageBuffer);
		input.counts.push_back(1);
		input.stages.push_back(vk::ShaderStageFlagBits::eVertex);

		descriptorSetLayouts[Vulkan::RenderStageType::Forward] = VkInit::MakeDescriptorSetLayout(Engine::GetInstance().GetLogicalDevice(), input);
	}
}

void VkInit::DestroyStageDescriptorSetLayouts()
{
	for (auto& [key,descriptorSetLayout] : descriptorSetLayouts)
	{
		Engine::GetInstance().GetLogicalDevice().destroyDescriptorSetLayout(descriptorSetLayout);
	}
}

vk::DescriptorSetLayout VkInit::GetStageDescriptorSetLayout(const Vulkan::RenderStageType aStage)
{
	return descriptorSetLayouts.at(aStage);
}

vk::DescriptorSetLayout VkInit::MakeDescriptorSetLayout(const vk::Device aDevice, const DescriptorSetLayoutData& aInput)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(aInput.count);

	for (size_t i = 0; i < aInput.count; i++)
	{
		vk::DescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = aInput.indices[i];
		layoutBinding.descriptorType = aInput.types[i];
		layoutBinding.descriptorCount = aInput.counts[i];
		layoutBinding.stageFlags = aInput.stages[i];

		layoutBindings.push_back(layoutBinding);
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.flags = vk::DescriptorSetLayoutCreateFlags();
	layoutInfo.bindingCount = aInput.count;
	layoutInfo.pBindings = layoutBindings.data();

	try
	{
		return aDevice.createDescriptorSetLayout(layoutInfo);
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to create DescriptorSetLayout, %s", err.what());
	}

	return {};
}

vk::DescriptorPool VkInit::MakeDescriptorPool(const vk::Device aDevice, const uint32_t aSize, const DescriptorSetLayoutData& aInput)
{
	std::vector<vk::DescriptorPoolSize> poolSizes;

	for (size_t i = 0; i < aInput.count; i++)
	{
		vk::DescriptorPoolSize poolSize{};
		poolSize.type = aInput.types[i];
		poolSize.descriptorCount = aSize;
		poolSizes.push_back(poolSize);
	}

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo.flags = aInput.flags;
	poolInfo.maxSets = aSize;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	try
	{
		return aDevice.createDescriptorPool(poolInfo);
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to create DescriptorPool, %s", err.what());
	}

	return vk::DescriptorPool();
}

vk::DescriptorSet VkInit::AllocateDescriptorSet(const vk::Device aDevice, const vk::DescriptorPool aDescriptorPool, const vk::DescriptorSetLayout aLayout)
{
	vk::DescriptorSetAllocateInfo allocateInfo;

	allocateInfo.descriptorPool = aDescriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &aLayout;

	try
	{
		return aDevice.allocateDescriptorSets(allocateInfo)[0];
	}
	catch(vk::SystemError err)
	{
		LOG_ERROR("Failed to allocate DescriptorSet, %s", err.what());
	}

	return vk::DescriptorSet();
}

void VkInit::VulkanDescriptorSet::Init(const vk::DescriptorPool aDescriptorPool, const vk::DescriptorSetLayout aLayout)
{
	Engine& engine = Engine::GetInstance();
	
	for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); ++i)
	{
		myDescriptorSets.push_back(AllocateDescriptorSet(engine.GetLogicalDevice(), aDescriptorPool, aLayout));
	}
}

vk::DescriptorSet VkInit::VulkanDescriptorSet::GetDescriptorSet() const
{
	return myDescriptorSets[Engine::GetInstance().GetCurrentSwapchainIndex()];
}

const std::vector<vk::DescriptorSet>& VkInit::VulkanDescriptorSet::GetAllDescriptorSets() const
{
	return myDescriptorSets;
}

void VkInit::DescriptorWriter::AddDescriptorInfo(const VulkanDescriptorSet& aDescriptorSet, const Vulkan::VulkanBuffer& aBuffer, 
	const vk::DescriptorType aType, const uint32_t aBinding)
{
	Engine& engine = Engine::GetInstance();
	myWriteOperations.resize(engine.GetSwapchainFrames().size());

	for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); ++i)
	{
		vk::WriteDescriptorSet writeInfo{};

		writeInfo.dstSet = aDescriptorSet.GetAllDescriptorSets()[i];
		writeInfo.dstBinding = aBinding;
		writeInfo.dstArrayElement = 0;
		writeInfo.descriptorCount = 1;
		writeInfo.descriptorType = aType;
		writeInfo.pBufferInfo = &aBuffer.GetAllDescriptorBufferInfos()[i];
		
		myWriteOperations[i].push_back(writeInfo);
	}
}

void VkInit::DescriptorWriter::UpdateDescriptorSets() const
{
	Engine& engine = Engine::GetInstance();
	engine.GetLogicalDevice().updateDescriptorSets(myWriteOperations[engine.GetCurrentSwapchainIndex()], {});
}

void VkInit::DescriptorWriter::Reset()
{
	myWriteOperations.clear();
}
