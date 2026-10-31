#pragma once

#include <algorithm>
#include <set>

#include "Component.h"
#include "Defines.h"

// ReSharper disable once CppWrongIncludesOrder
#include "ArchetypeManager.h"
#include "Id/IdTypes.h"
#include "Components/TransformComponent.h"
// ReSharper disable once CppWrongIncludesOrder
//#include <WinPixEventRuntime/Instrumenation.h>

#include "ECSInstance.h"
#include "Util/Logging/Logging.h"

namespace ECS
{
	namespace Internal
	{
		struct FutureComponentsArray
		{
			std::byte* data;
			size_t componentSize;

			EntityID* entityIDs;

			size_t componentCount;
			size_t capacity;
		};

		static EntityID globalNextFreeEntityID{120u << 24};
	}

	class Entity
	{
	public:
		Entity();
		Entity(EntityID aEntityID);

		EntityID GetID() const;
		EntityID GetParentId() const;
		
		template<class T>
		[[nodiscard]]T* GetComponent() const;

		template<class T>
		bool HasComponent() const;
		
		template<class T>
		bool HasComponentInArchetype() const;
		
		Goose::TransformComponent* GetTransform() const;

		// const Goose::TransformComponent* GetTransform() const;

		template <class Component, class ... Arguments>
		void WriteComponentData(Arguments... aComponentArguments, Archetype* aArchetype, size_t aComponentIndex);

		template<class Component, class ...Arguments>
		Component* AddComponent(Arguments...);
		
		template<class Component>
		bool RemoveComponent();
		
		bool IsValid() const;

		bool IsStatic() const;

		bool IsActive() const;
		void SetIsActive(bool aIsActive) const;
		
		void SetLayer(Wonder::LayerIndex aLayer);

		Wonder::LayerIndex GetLayer() const;

		const char* GetName() const;

		bool operator==(const Entity& aEntity) const;

		static void FlushFutures();

		static void CreateFutureArray(ComponentID aID, size_t aSize);

	private:
		EntityID myEntityID;

		static std::unordered_map<ComponentID, Internal::FutureComponentsArray> ourFutureComponents;
		static std::unordered_map<ComponentID, std::vector<EntityID>> ourFutureRemovedComponents;

		static void MoveComponentData(EntityID aEntityID, Archetype* oldArchetype, Archetype* newArchetype, const std::set<ComponentID>& aComponentIDsToMove);

		static void FlushFutureComponents();
		static void FlushFutureRemovedComponents();
	};

	Entity CreateEntity(bool aIsStatic = false, Wonder::LayerIndex aLayer = 0, std::string_view aName = "Undefined", EntityID aParentId = INVALID_ENTITY);
	Entity CreateEntity(bool aIsStatic, Wonder::LayerIndex aLayer, EntityID aID, std::string_view aName = "Undefined", EntityID aParentId = INVALID_ENTITY);
	void DeleteEntity(Entity aEntity);
	void DeleteEntity(EntityID aEntity);
	void DeleteAllEntities();
	Entity GetEntityFromID(EntityID aID);
	size_t GetEntityCount();
	bool EntityExists(EntityID aID);

	template<typename ...Components>
	bool FindEntity(Entity& aEntityOut);

	EntityRecord& GetEntityRecord(EntityID aEntityID);

	template<typename T, typename ...Arguments>
	void OnComponentCreate(T& aComponent, const Arguments&...);

	template<typename T, typename ...Arguments>
	void OnComponentCreate(ECS::Entity aEntity, T& aComponent, const Arguments&...);

	template<typename T>
	void OnComponentDelete(T& aComponent);

	template<typename T>
	void OnComponentDelete(ECS::Entity aEntity, T& aComponent);
}

namespace ECS
{
	template <class T>
	T* Entity::GetComponent() const
	{
		if (IsValid() == false)
		{
			LOG_ERROR("Tried to get a component from an invalid entity");
			return nullptr;
		}

		const EntityRecord& record = GetEntityRecord(myEntityID);
		if (record.archetype == nullptr)
		{
			return nullptr;
		}

		auto matchingComponent = record.archetype->componentArrays.find(GetComponentID<T>());
		if (matchingComponent == record.archetype->componentArrays.end())
		{
			const Internal::FutureComponentsArray& componentArray = ourFutureComponents[GetComponentID<T>()];
			for (unsigned int i = 0; i < componentArray.componentCount; ++i)
			{
				unsigned int entityID = componentArray.entityIDs[i];
				if (myEntityID == entityID)
				{
					return reinterpret_cast<T*>(componentArray.data + componentArray.componentSize * i);
				}
			}
			return nullptr;
		}

		ComponentArray& column = matchingComponent->second;
		size_t entityIndex = record.archetype->entityIDToIndex[myEntityID];
		return reinterpret_cast<T*>(column.data + entityIndex * column.componentSize);
	}

	template <class T>
	bool Entity::HasComponent() const
	{
		if (IsValid() == false)
		{
			LOG_ERROR("Tried to check if an invalid entity had a component");
			return false;
		}

		const EntityRecord& record = GetEntityRecord(myEntityID);
		if (record.archetype == nullptr)
		{
			return false;
		}

		if (record.archetype->componentSet.contains(GetComponentID<T>()))
		{
			return true;
		}
		
		const Internal::FutureComponentsArray& componentArray = ourFutureComponents[GetComponentID<T>()];
		for (unsigned int i = 0; i < componentArray.componentCount; ++i)
		{
			if (myEntityID == componentArray.entityIDs[i])
			{
				return true;
			}
		}
		return false;
	}

	template <class T>
	bool Entity::HasComponentInArchetype() const
	{
		if (IsValid() == false)
		{
			LOG_ERROR("Tried to check if an invalid entity had a component");
			return false;
		}

		const EntityRecord& record = GetEntityRecord(myEntityID);
		if (record.archetype == nullptr)
		{
			return false;
		}

		return record.archetype->componentSet.contains(GetComponentID<T>());
	}

	template <class Component, class ... Arguments>
	void Entity::WriteComponentData(Arguments... aComponentArguments, Archetype* aArchetype, size_t aComponentIndex)
	{
		const ComponentID newComponentID = GetComponentID<Component>();
		
		ComponentArray& componentArray = aArchetype->componentArrays[newComponentID];
		Component* newDataPointer = reinterpret_cast<Component*>(componentArray.data + aComponentIndex * componentArray.componentSize);
		new (newDataPointer) Component{aComponentArguments...};
	}

	template <class Component, class ... Arguments>
	Component* Entity::AddComponent(Arguments... aComponentArguments)
	{
		if (IsValid() == false)
		{
			LOG_ERROR("Tried to add a component from an invalid entity");
			return nullptr;
		}

		const ComponentID newComponentID = GetComponentID<Component>();

		Internal::FutureComponentsArray& componentArray = ourFutureComponents[newComponentID];
		if (std::find(componentArray.entityIDs, componentArray.entityIDs + componentArray.componentCount, myEntityID) != componentArray.entityIDs + componentArray.componentCount)
		{
			return nullptr;
		}

		const EntityRecord& record = GetEntityRecord(myEntityID);
		Archetype* oldArchetype = record.archetype;
		if (oldArchetype->componentSet.contains(newComponentID))
		{
			return nullptr;
		}

		// increase capacity if needed
		if (componentArray.componentCount >= componentArray.capacity)
		{
			const size_t newCapacity = static_cast<size_t>(componentArray.capacity * COMPONENT_FUTURES_SIZE_INCREASE_SCALAR);

			std::byte* oldData = componentArray.data;
			componentArray.data = new std::byte[newCapacity * componentArray.componentSize];
			const ComponentInfo* componentInfo = GetComponentInfo(newComponentID);
			for (size_t i = 0; i < componentArray.capacity; ++i)
			{
				componentInfo->moveConstructor(componentArray.data + i * componentArray.componentSize, oldData + i * componentArray.componentSize);
				componentInfo->destructor(oldData + i * componentArray.componentSize);
			}
			
			delete oldData;

			EntityID* oldEntityIDs = componentArray.entityIDs;
			componentArray.entityIDs = new EntityID[newCapacity];
			std::memcpy(componentArray.entityIDs, oldEntityIDs, componentArray.capacity * sizeof(EntityID));

			delete oldEntityIDs;

			componentArray.capacity = newCapacity;
		}

		Component* dataPointer = reinterpret_cast<Component*>(componentArray.data + componentArray.componentCount * componentArray.componentSize);
		if constexpr (sizeof...(Arguments) == 1)
		{
			if constexpr (std::is_same_v<Component, Arguments...>)
			{
				new (dataPointer) Component{ aComponentArguments... };
			}
			else
			{
				new (dataPointer) Component{};
			}
		}
		else
		{
			new (dataPointer) Component{ aComponentArguments... };
		}
		
		ECS::OnComponentCreate(*dataPointer, aComponentArguments...);
		ECS::OnComponentCreate(static_cast<Entity>(myEntityID), *dataPointer, aComponentArguments...);
		componentArray.entityIDs[componentArray.componentCount] = myEntityID;
		++componentArray.componentCount;

		return dataPointer;
	}

	template <class Component>
	bool Entity::RemoveComponent()
	{
		if (IsValid() == false)
		{
			LOG_ERROR("Tried to remove component from an invalid entity");
			return false;
		}

		const ComponentID removedComponentID = GetComponentID<Component>();

		std::vector<EntityID>& toRemoveComponents = ourFutureRemovedComponents[removedComponentID];
		if (std::ranges::find(toRemoveComponents, myEntityID) != toRemoveComponents.end())
		{
			return false;
		}

		const EntityRecord& record = GetEntityRecord(myEntityID);
		Archetype* oldArchetype = record.archetype;
		if (oldArchetype->componentSet.contains(removedComponentID) == false)
		{
			return false;
		}

		toRemoveComponents.emplace_back(myEntityID);
		Component* component = GetComponent<Component>();

		ECS::OnComponentDelete(*component);
		ECS::OnComponentDelete(static_cast<Entity>(myEntityID), *component);

		return true;
	}

	template <typename ... Components>
	bool FindEntity(Entity& aEntityOut)
	{
		if (globalCurrentECS == nullptr)
		{
			return false;
		}

		std::vector<ComponentID> componentIDs{ ECS::GetComponentID<Components>()... };

		for (Archetype* archetype : globalCurrentECS->archetypeManager.GetArchetypes())
		{
			if (globalCurrentECS->archetypeManager.ArchetypeHasComponents(archetype, componentIDs))
			{
				if (archetype->entityCount == 0)
				{
					continue;
				}

				aEntityOut = archetype->indexToEntityID[0];
				return true;
			}
		}

		return false;
	}

	template <typename T>
	void OnReflectComponent(T&)
	{
		__noop;
	}
	template <typename T>
	void OnComponentDataSet(T&)
	{
		__noop;
	}

	template <typename T>
	void OnComponentDataSet(ECS::Entity, T&)
	{
		__noop;
	}

	template <typename T, typename ...Arguments>
	void OnComponentCreate(T&, const Arguments&...)
	{
		__noop;
	}

	template <typename T, typename ... Arguments>
	void OnComponentCreate(ECS::Entity, T&, const Arguments&...)
	{
		__noop;
	}

	template <typename T>
	void OnComponentDelete(T&)
	{
		__noop;
	}

	template <typename T>
	void OnComponentDelete(ECS::Entity, T&)
	{
		__noop;
	}
}
