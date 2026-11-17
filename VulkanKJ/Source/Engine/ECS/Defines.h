#pragma once

#include <string>
#include <vector>
#include <functional>
#include "Id/IdTypes.h"
#include "Components/TransformComponent.h"

namespace ECS
{
	struct Archetype;
	
	using ComponentID = uint32_t;
	using ArchetypeID = uint32_t;
	using EntityID = uint64_t;
	
	constexpr EntityID INVALID_ENTITY = 0;
	
	struct ComponentInfo
	{
		std::vector<ArchetypeID> archetypeUses;
		std::string name;
		size_t size;
		std::function<void(void*)> destructor;
		std::function<void(void*, void*)> moveConstructor;
	};
	
	struct EntityRecord
	{
		Archetype* archetype;
		std::string name;

		bool isStatic = false;
		bool isActive = true;
		bool isParent = false;
		Wonder::LayerIndex layer{};
		EntityID parent = INVALID_ENTITY;
		Wonder::PrefabObjectId prefabObjectId;
	};
	
	struct EntityParentInfo
	{
		std::vector<EntityID> children;
		Goose::TransformComponent lastTransform;
	};
	
	struct EntityReference
	{
		EntityID headEntityId;
		Wonder::PrefabObjectId prefabObjectId;
		
		bool operator==(const EntityReference& other) const
		{
			return prefabObjectId == other.prefabObjectId && headEntityId == other.headEntityId;
		}
	};

	static constexpr float ARCHETYPE_SIZE_INCREASE_SCALAR = 2;
	static constexpr float COMPONENT_FUTURES_SIZE_INCREASE_SCALAR = 2;
}

template<>
struct std::hash<ECS::EntityReference>
{
	std::size_t operator()(const ECS::EntityReference& aReference) const noexcept
	{
		return std::hash<ECS::EntityID>{}(aReference.headEntityId) * std::hash<Wonder::PrefabObjectId>{}(aReference.prefabObjectId);
	}
};
