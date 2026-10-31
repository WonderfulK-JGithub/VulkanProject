#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include <nlohmann/json.hpp>

#include "Vulkan/VkMesh.h"
#include "ID/IdTypes.h"
#include "Rendering/RenderImage.h"
#include "Rendering/ResolutionData.h"
#include "Shaders/ShaderReflection.h"
#include "Util/StringRegistry.h"
#include "Util/Container/FixedString.h"
#include "Vulkan/VkInit/Descriptors.h"



namespace Wonder
{
	enum class MaterialType
	{
		Skybox,
		Mesh,
		Line,
		Fullscreen,
	};
	
	struct PushConstantProperty
	{
		ShaderReflection::PushConstantInfo info;
		Goose::String32 name;
	};
	
	class MaterialAsset
	{
	public:	
		void Init(const nlohmann::json& aJson);
		void SaveToJson(nlohmann::json& aJson);
		
		void CopyFromOther(const MaterialAsset& aOther);
		void AdjustPushConstantsToReflection();
		
		void InitEngine(MaterialId aId);
		
		void AddTexture(Wonder::TextureId aTextureId);
		void AddRenderImage(RenderImageId aImageId);
		void RemoveRenderImages();

		Tga::StringId& AccessVertexShaderName();
		Tga::StringId& AccessFragmentShaderName();
		std::vector<PushConstantProperty>& AccessPushConstants();
		
		void SetRenderStage(Vulkan::RenderStageType aRenderStage);
		void SetMaterialType(const MaterialType& aMaterialType);
		
		void Build();
		void Bind(vk::CommandBuffer aCommandBuffer,Vulkan::ResolutionId aResolutionId) const;
		void BuildOrRebuildGraphicsPipeline(Vulkan::ResolutionId aResolutionId,vk::Extent2D aResolution);
		void DestroyGraphicsPipeline(Vulkan::ResolutionId aResolutionId);
		
		void Destroy();
		
		vk::PipelineLayout GetPipelineLayout() const;
	private:
		
		Tga::StringId myVertexShaderName;
		Tga::StringId myFragmentShaderName;
		
		std::vector<PushConstantProperty> myPushConstants;

		vk::PipelineLayout myGraphicsPipelineLayout;
		std::unordered_map<Vulkan::ResolutionId,vk::Pipeline> myGraphicsPipelines;
		
		Vulkan::RenderStageType myRenderStage{};
		MaterialType myMaterialType{};
		
		MaterialId myId;
	};
}
