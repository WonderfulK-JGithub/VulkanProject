#pragma once
#include "AssetManagement/AssetRegistry.h"
#include "Vulkan/VkMesh.h"
#include "Util/Container/MappedVector.hpp"
#include "Vulkan/VkInit/Swapchain.h"

namespace Wonder
{
	struct BatchId
	{
		AssetIndex materialIndex;
		Vulkan::MeshId meshId;
		
		bool operator==(const BatchId& aOther) const
		{
			return materialIndex == aOther.materialIndex && meshId == aOther.meshId;
		}
	};
}

template<>
struct std::hash<Wonder::BatchId>
{
	std::size_t operator()(const Wonder::BatchId& aId) const noexcept
	{
		return std::hash<Wonder::AssetIndex>{}(aId.materialIndex) * std::hash<Vulkan::MeshId>{}(aId.meshId);
	}
};

namespace Wonder
{
	class MaterialAsset;
	
	class ModelInstanceContainer
	{
	public:
		void AddInstance(MaterialId aMaterialId, Vulkan::MeshId aMeshId, const VkInit::ModelInstance& aInstance);

		void ClearInstances();

		const Goose::MappedVector<BatchId,std::vector<VkInit::ModelInstance>>& GetInstances() const;

		void Reset();

	private:
		Goose::MappedVector<BatchId,std::vector<VkInit::ModelInstance>> myInstances;
	};
	
	
}