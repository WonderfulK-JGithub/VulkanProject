#pragma once
#include <vulkan/vulkan.hpp>
#include "../RenderStructs.h"
#include "../VkMesh.h"
namespace VkInit
{
	enum class PipelineType
	{
		Cubemap,
		Default,
	};

	struct GraphicsPipelineInBundle
	{
		vk::Device device;
		vk::PipelineLayout layout;
		std::string vertexShaderfilePath;
		std::string fragmentShaderfilePath;
		
		vk::Extent2D renderSize;
		vk::Format colorFormat;
		vk::Format depthFormat;

		Vulkan::RenderStageType stage;
		Vulkan::VertexLayoutType vertexType;
	};

	vk::Pipeline MakeGraphicsPipeline(const GraphicsPipelineInBundle& aSpecification);

	namespace Internal
	{
		vk::PipelineInputAssemblyStateCreateInfo MakeDefaultInputAssemblyInfo();
		vk::PipelineInputAssemblyStateCreateInfo MakeLineInputAssemblyInfo();

		vk::Viewport MakeViewport(const GraphicsPipelineInBundle& aSpecification);

		vk::Rect2D MakeScissor(const GraphicsPipelineInBundle& aSpecification);

		vk::PipelineViewportStateCreateInfo MakeViewportStateInfo(const vk::Viewport& aViewport, const vk::Rect2D& aScissor);

		vk::PipelineRasterizationStateCreateInfo MakeRasterizerInfo();

		vk::PipelineShaderStageCreateInfo MakeShaderInfo(const vk::ShaderModule& aShaderModule,const vk::ShaderStageFlagBits& aStage);

		vk::PipelineMultisampleStateCreateInfo MakeMultisampleInfo();

		vk::PipelineColorBlendStateCreateInfo MakeColorBlendStateInfo(const vk::PipelineColorBlendAttachmentState& aColorBlendAttachment);
	}
	
}