#include "ArchetypeManager.h"

#include <ranges>

#include "Component.h"
#include "Entity.h"

namespace ECS
{
	Archetype& ArchetypeManager::CreateArchetype(const std::set<ComponentID>& aComponents)
	{
		static constexpr size_t ARCHETYPE_START_CAPACITY = 2;

		const ArchetypeID archetypeID = myNextArchetypeID++;

		Archetype* const newArchetype = new Archetype{
			.archetypeID = archetypeID,
			.componentSet = aComponents,
			.entityCount = 0,
			.capacity = ARCHETYPE_START_CAPACITY,
			.componentArrays = std::unordered_map<ComponentID, ComponentArray>{},
			.entityIDToIndex = std::unordered_map<EntityID, size_t>{},
			.indexToEntityID = std::vector<EntityID>{},
			.archetypeEdgeUp = std::unordered_map<ComponentID, Archetype*>{},
			.archetypeEdgeDown = std::unordered_map<ComponentID, Archetype*>{}
		};

		for (ComponentID componentID : aComponents)
		{
			ComponentInfo* const componentInfo = GetComponentInfo(componentID);

			if (componentInfo->archetypeUses.empty())
			{
				componentInfo->archetypeUses.emplace_back(archetypeID);
			}
			else
			{
				for (size_t i = 0; i < componentInfo->archetypeUses.size(); ++i)
				{
					if (componentInfo->archetypeUses[i] == archetypeID)
					{
						break;
					}

					if (componentInfo->archetypeUses[i] > archetypeID)
					{
						componentInfo->archetypeUses.insert(componentInfo->archetypeUses.begin() + i, archetypeID);
						break;
					}
				}
			}

			const auto data = new std::byte[ARCHETYPE_START_CAPACITY * componentInfo->size];
			newArchetype->componentArrays.emplace(componentID, ComponentArray{data, componentInfo->size});
		}
		
		myArchetypes.push_back(newArchetype);
		++myLastCreationTime;
		return *newArchetype;
	}

	void ArchetypeManager::IncreaseArchetypeCapacity(Archetype* aArchetype)
	{
		const size_t newCapacity = static_cast<size_t>(static_cast<float>(aArchetype->capacity) * ARCHETYPE_SIZE_INCREASE_SCALAR);
		for (auto& componentID : aArchetype->componentSet)
		{
			ComponentArray& componentArray = aArchetype->componentArrays[componentID];
			const ComponentInfo* componentInfo = GetComponentInfo(componentID);
			
			std::byte* oldData = componentArray.data;
			componentArray.data = new std::byte[newCapacity * componentArray.componentSize];
			for (size_t i = 0; i < aArchetype->capacity; ++i)
			{	
				componentInfo->moveConstructor(componentArray.data + i * componentArray.componentSize, oldData + i * componentArray.componentSize);
				componentInfo->destructor(oldData + i * componentArray.componentSize);
			}
			
			delete oldData;
		}
		
		aArchetype->capacity = newCapacity;
	}

	void ArchetypeManager::RemoveEntityFromArchetype(Entity aEntity)
	{
		EntityID id = aEntity.GetID();
		EntityRecord& entityRecord = GetEntityRecord(id);
		Archetype* archetype = entityRecord.archetype;

		--archetype->entityCount;
		
		size_t entityIndex = archetype->entityIDToIndex.at(id);
		size_t lastIndex = archetype->entityCount;
		
		for (auto [componentID, componentArray] : archetype->componentArrays)
		{
			const ComponentInfo* componentInfo = GetComponentInfo(componentID);
			componentInfo->destructor(componentArray.data + entityIndex * componentArray.componentSize);
			
			componentInfo->moveConstructor(
				componentArray.data + entityIndex * componentArray.componentSize,
				componentArray.data + lastIndex * componentArray.componentSize);
			componentInfo->destructor(componentArray.data + lastIndex * componentArray.componentSize);
		}

		const EntityID lastEntityID = archetype->indexToEntityID[lastIndex];
		archetype->entityIDToIndex[lastEntityID] = archetype->entityIDToIndex[id];
		archetype->indexToEntityID[entityIndex] = lastEntityID;

		archetype->entityIDToIndex.erase(id);
		archetype->indexToEntityID.pop_back();
	}

	std::vector<Archetype*>& ArchetypeManager::GetArchetypes()
	{
		return myArchetypes;
	}

	bool ArchetypeManager::ArchetypeHasComponents(const Archetype* aArchetype, const std::vector<ComponentID>& aComponents)
	{
		for (ComponentID componentID : aComponents)
		{
			if (aArchetype->componentSet.contains(componentID) == false)
			{
				return false;
			}
		}

		return true;
	}
}
