#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include "VkMemory.h"
#include <unordered_map>
#include <glm/glm.hpp>

#include "Util/Container/MappedVector.hpp"
#include "Util/FileManagement/FileTypes.h"

namespace Vulkan
{
	struct PosColorVertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 normal;
	};
	
	struct LineVertex
	{
		glm::vec3 position;
		glm::vec4 color;
	};

	enum class VertexLayoutType
	{
		Empty,
		PosColor,
		Line
	};
	
	using MeshId = uint16_t;
	
	struct MeshData
	{
		std::vector<PosColorVertex> vertices;
		std::vector<uint32_t> indices;
		MeshId id{};
	};

	void CreateVertexInputInfos();
	vk::PipelineVertexInputStateCreateInfo GetVertexInputInfo(VertexLayoutType aType);

	vk::VertexInputBindingDescription GetPosColorBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> GetPosColorAttributeDescriptions();
	
	vk::VertexInputBindingDescription GetLineBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> GetLinerAttributeDescriptions();

	struct FinalizeInput
	{
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		vk::Queue queue;
		vk::CommandBuffer commandBuffer;
	};

	struct VertexDrawInfo
	{
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t vertexOffset;
	};
	
	class MeshPool
	{
	public:
		MeshPool();
		~MeshPool();
		
		static MeshPool& GetInstance();

		// void AddMesh(MeshType aType, const std::vector<PosColorVertex>& aVertices, const std::vector<uint32_t>& aIndices);
		void AddMesh(MeshData& aMeshData);
		
		void RemoveMesh(MeshId aId);
		
		void Finalize(const FinalizeInput& aInput);
		bool NeedsUpdate() const;

		VertexDrawInfo GetVertexDrawInfo(MeshId aId) const;

		Vulkan::Buffer myVertexBuffer;
		Vulkan::Buffer myIndexBuffer;
	private:
		
		std::unordered_map<MeshId, uint32_t> firstIndices;
		std::unordered_map<MeshId, uint32_t> vertexOffsets;
		std::unordered_map<MeshId, uint32_t> indexCounts;
		Goose::MappedVector<MeshId,MeshData*> myMeshData;
		
		vk::Device myLogicalDevice;
		
		bool myNeedsUpdate{};
		bool myHasBeenInitialized{};
	};
}
