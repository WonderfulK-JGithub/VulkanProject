#include "Entity.h"

#include <iostream>
#include <ranges>

#include "Util/Container/MappedVector.hpp"


std::unordered_map<ECS::ComponentID, ECS::Internal::FutureComponentsArray> ECS::Entity::ourFutureComponents{};
std::unordered_map<ECS::ComponentID, std::vector<ECS::EntityID>> ECS::Entity::ourFutureRemovedComponents{};

namespace 
{
    std::vector<ECS::EntityID> locToEraseEntities;
    std::vector<ECS::EntityID> locToEmplaceEntities;
}


namespace
{
    void EmplaceEntity(ECS::EntityID aId)
    {
	    ECS::Archetype& baseArchetype = ECS::globalCurrentECS->archetypeManager.GetArchetype(std::set<ECS::ComponentID>{});
        baseArchetype.entityIDToIndex.emplace(aId, baseArchetype.entityCount);
        baseArchetype.indexToEntityID.emplace_back(aId);
        baseArchetype.entityCount++;
        if (baseArchetype.capacity >= baseArchetype.entityCount)
        {
            baseArchetype.capacity *= 2;
        }
    }

    void EraseEntity(ECS::EntityID aId)
    {
        if (reinterpret_cast<ECS::Entity*>(&aId)->IsValid() == false) return;
	    ECS::globalCurrentECS->archetypeManager.RemoveEntityFromArchetype(aId);
        ECS::globalCurrentECS->entityIndex.erase(aId);
    }
}
namespace ECS
{
    Entity::Entity()
	    : myEntityID(INVALID_ENTITY)
    {
    }

    Entity::Entity(const EntityID aEntityID): myEntityID(aEntityID)
    {
    }

    EntityID Entity::GetID() const
    {
        return myEntityID;
    }

    EntityID Entity::GetParentId() const
    {
        return GetEntityRecord(myEntityID).parent;
    }

    bool Entity::IsValid() const
    {
        if (myEntityID == INVALID_ENTITY)
        {
            return false;
        }
        
        return globalCurrentECS->entityIndex.contains(myEntityID);
    }

    bool Entity::IsStatic() const
    {
        if (IsValid() == false)
        {
            LOG_ERROR("Tried to get the static state of an invalid entity");
            return false;
        }

        return GetEntityRecord(myEntityID).isStatic;
    }

    bool Entity::IsActive() const
    {
        if (IsValid() == false)
        {
            LOG_ERROR("Tried to get the active state of an invalid entity");
            return false;
        }

        return GetEntityRecord(myEntityID).isActive;
    }

    void Entity::SetIsActive(const bool aIsActive) const
    {
        if (IsValid() == false)
        {
            LOG_ERROR("Tried to set the active state of an invalid entity");
            return;
        }

        GetEntityRecord(myEntityID).isActive = aIsActive;
    }

    void Entity::SetLayer(Wonder::LayerIndex aLayer)
    {
        if (IsValid() == false)
        {
            LOG_ERROR("Tried to get the layer of an invalid entity");
            return;
        }

        GetEntityRecord(myEntityID).layer = aLayer;
    }

    Wonder::LayerIndex Entity::GetLayer() const
    {
        if (IsValid() == false)
        {
            LOG_ERROR("Tried to get the layer of an invalid entity");
            return 0;
        }

        return GetEntityRecord(myEntityID).layer;
    }

    // Goose::TransformComponent* Entity::GetTransform()
    // {
    //     return const_cast<Goose::TransformComponent*>(const_cast<const Entity*>(this)->GetTransform());
    // }

    Goose::TransformComponent* Entity::GetTransform() const
    {
        if (IsValid() == false)
        {
            LOG_ERROR("Tried to get the transform from an invalid entity");
            return nullptr;
        }

        const EntityRecord& record = GetEntityRecord(myEntityID);
        auto matchingComponent = record.archetype->componentArrays.find(GetComponentID<Goose::TransformComponent>());

        ComponentArray& column = matchingComponent->second;
        size_t entityIndex = record.archetype->entityIDToIndex[myEntityID];
        return reinterpret_cast<Goose::TransformComponent*>(column.data + entityIndex * column.componentSize);
    }

    const char* Entity::GetName() const
    {
        return ECS::globalCurrentECS->entityIndex[myEntityID].name.c_str();
    }
    

    bool Entity::operator==(const Entity& aEntity) const
    {
        return myEntityID == aEntity.myEntityID;
    }

    Entity CreateEntity(const bool aIsStatic, const Wonder::LayerIndex aLayer, const std::string_view aName, const EntityID aParentId)
    {
    	return CreateEntity(aIsStatic, aLayer, Internal::globalNextFreeEntityID++, aName, aParentId);
    }

    Entity CreateEntity(const bool aIsStatic, const Wonder::LayerIndex aLayer, const EntityID aID, const std::string_view aName, const EntityID aParentId)
    {
        ComponentID transformID = ECS::GetComponentID<Goose::TransformComponent>();
        Archetype& archetype = globalCurrentECS->archetypeManager.GetArchetype(std::set{ transformID });
        if (globalCurrentECS->entityIndex.contains(aID))
        {
            LOG_WARNING("Tried to create an entity with an already existing ID! old: %s new: %s", globalCurrentECS->entityIndex[aID].name.data(), aName.data());
        }
        
        ECS::globalCurrentECS->entityIndex.emplace(aID, EntityRecord{
            .archetype = &archetype,
            .name = aName.data(),
            .isStatic = aIsStatic,
            .isActive = true,
            .layer = aLayer,
            .parent = aParentId
        });

        if (archetype.entityCount >= archetype.capacity)
        {
            globalCurrentECS->archetypeManager.IncreaseArchetypeCapacity(&archetype);
        }

        size_t newComponentIndex = archetype.entityCount;

        // write data
        ComponentArray& transformArray = archetype.componentArrays[transformID];
        new (transformArray.data + newComponentIndex * transformArray.componentSize) (Goose::TransformComponent){};

        // update lookup tables
        archetype.entityIDToIndex.emplace(aID, newComponentIndex);
        archetype.indexToEntityID.emplace_back(aID);
        ++archetype.entityCount;

        return aID;
    }
	
    void DeleteEntity(const Entity aEntity)
    {
        DeleteEntity(aEntity.GetID());
    }

    void DeleteEntity(EntityID aEntity)
    {
        if (std::ranges::find(locToEraseEntities, aEntity) != locToEraseEntities.end())
        {
            LOG_WARNING("Tried to delete a already deleted entity!");
            return;
        }

        locToEraseEntities.emplace_back(aEntity);
    }

    void DeleteAllEntities()
    {
	    for (size_t i = 0; i < ECS::globalCurrentECS->entityIndex.size(); ++i)
	    {
            EntityID dataKey = ECS::globalCurrentECS->entityIndex.dataKeys()[i];

            if (std::ranges::find(locToEraseEntities, dataKey) != locToEraseEntities.end())
            {
                LOG_WARNING("Tried to delete a already deleted entity!");
                continue;
            }

            locToEraseEntities.emplace_back(dataKey);
	    }
    }

    Entity GetEntityFromID(EntityID aID)
    {
        return {aID};
    }

    size_t GetEntityCount()
    {
        return ECS::globalCurrentECS->entityIndex.size();
    }

    bool EntityExists(const EntityID aID)
    {
		if (globalCurrentECS == nullptr)
		{
            return false;
		}
        return ECS::globalCurrentECS->entityIndex.contains(aID);
    }

    EntityRecord& GetEntityRecord(const EntityID aEntityID)
    {
        return ECS::globalCurrentECS->entityIndex.at(aEntityID);
    }
    
    void Entity::MoveComponentData(const EntityID aEntityID, Archetype* oldArchetype, Archetype* newArchetype, const std::set<ComponentID>& aComponentIDsToMove)
    {
        const size_t newComponentIndex = newArchetype->entityCount;
        const size_t oldComponentIndex = oldArchetype->entityIDToIndex[aEntityID];
        const size_t oldArchetypeLastIndex = oldArchetype->entityCount - 1;
			
        for (ComponentID componentID : aComponentIDsToMove)
        {
            const ComponentArray& oldColumn = oldArchetype->componentArrays[componentID];
            const ComponentArray& newColumn = newArchetype->componentArrays[componentID];
            
            ComponentInfo* componentInfo = GetComponentInfo(componentID);

            componentInfo->moveConstructor(
                newColumn.data + newComponentIndex * newColumn.componentSize,
                oldColumn.data + oldComponentIndex * oldColumn.componentSize);

            // filling hole code
            if (oldComponentIndex != oldArchetypeLastIndex)
            {
                componentInfo->moveConstructor(
                    oldColumn.data + oldComponentIndex * oldColumn.componentSize,
                    oldColumn.data + oldArchetypeLastIndex * newColumn.componentSize);
                componentInfo->destructor(oldColumn.data + oldArchetypeLastIndex * oldColumn.componentSize);
            }
        }
    }

    void Entity::FlushFutureComponents()
    {
	    for (auto& [componentID, futureComponentArray] : ourFutureComponents)
	    {
            for (unsigned int i = 0; i < futureComponentArray.componentCount; ++i)
            {
                EntityID entityID = futureComponentArray.entityIDs[i];
                const EntityRecord& record = GetEntityRecord(entityID);

                // get old and new archetype
                Archetype* oldArchetype = record.archetype;
                Archetype* newArchetype;
                auto archetypeEdge = oldArchetype->archetypeEdgeUp.find(componentID);
                if (archetypeEdge != oldArchetype->archetypeEdgeUp.end())
                {
                    newArchetype = archetypeEdge->second;
                }
                else
                {
                    std::set<ComponentID> components = oldArchetype->componentSet;
                    components.emplace(componentID);
                    newArchetype = &globalCurrentECS->archetypeManager.GetArchetype(components);
                    oldArchetype->archetypeEdgeUp.emplace(componentID, newArchetype);
                    newArchetype->archetypeEdgeDown.emplace(componentID, oldArchetype);
                }

                // increase capacity if needed
                if (newArchetype->entityCount >= newArchetype->capacity)
                {
                    globalCurrentECS->archetypeManager.IncreaseArchetypeCapacity(newArchetype);
                }

                size_t newComponentIndex = newArchetype->entityCount;

                // write data
                MoveComponentData(entityID, oldArchetype, newArchetype, oldArchetype->componentSet);
                ComponentArray& componentArray = newArchetype->componentArrays[componentID];
                const ComponentInfo* componentInfo = GetComponentInfo(componentID);
				componentInfo->moveConstructor(
				    componentArray.data + newArchetype->entityCount * componentArray.componentSize, 
                    futureComponentArray.data + i * futureComponentArray.componentSize);
                componentInfo->destructor(futureComponentArray.data + i * futureComponentArray.componentSize);

                // update lookup tables
                oldArchetype->entityIDToIndex[oldArchetype->indexToEntityID.back()] = oldArchetype->entityIDToIndex[entityID];
                oldArchetype->indexToEntityID[oldArchetype->entityIDToIndex[entityID]] = oldArchetype->indexToEntityID.back();
                oldArchetype->entityIDToIndex.erase(entityID);
                oldArchetype->indexToEntityID.pop_back();
                --oldArchetype->entityCount;

                newArchetype->entityIDToIndex.emplace(entityID, newComponentIndex);
                newArchetype->indexToEntityID.emplace_back(entityID);
                ++newArchetype->entityCount;

                // update record
                GetEntityRecord(entityID).archetype = newArchetype;
            }

            futureComponentArray.componentCount = 0;
	    }
    }

    void Entity::FlushFutureRemovedComponents()
    {
        for (auto& [componentID, entityIDs] : ourFutureRemovedComponents)
        {
            for (EntityID entityID : entityIDs)
            {
                //Entity en(entityID);

                // get old component set
                const EntityRecord& record = GetEntityRecord(entityID);
                Archetype* oldArchetype = record.archetype;

                Archetype* newArchetype;
                auto archetypeEdge = oldArchetype->archetypeEdgeDown.find(componentID);
                if (archetypeEdge != oldArchetype->archetypeEdgeDown.end())
                {
                    newArchetype = archetypeEdge->second;
                }
                else
                {
                    std::set<ComponentID> components = oldArchetype->componentSet;
                    components.erase(componentID);

                    newArchetype = &globalCurrentECS->archetypeManager.GetArchetype(components);
                    oldArchetype->archetypeEdgeDown.emplace(componentID, newArchetype);
                    newArchetype->archetypeEdgeUp.emplace(componentID, oldArchetype);
                }

                // increase capacity if needed
                if (newArchetype->entityCount >= newArchetype->capacity)
                {
                    globalCurrentECS->archetypeManager.IncreaseArchetypeCapacity(newArchetype);
                }

                size_t newComponentIndex = newArchetype->entityCount;

                // write data
                {
                    MoveComponentData(entityID, oldArchetype, newArchetype, newArchetype->componentSet);

                    // move removed component in old archetype (to fill in hole)
                    const size_t oldComponentIndex = oldArchetype->entityIDToIndex[entityID];
                    const size_t oldArchetypeLastIndex = oldArchetype->entityCount - 1;
                    const ComponentArray& oldColumn = oldArchetype->componentArrays[componentID];
                    const ComponentInfo* componentInfo = GetComponentInfo(componentID);
                    componentInfo->moveConstructor(
                        oldColumn.data + oldComponentIndex * oldColumn.componentSize,
                        oldColumn.data + oldArchetypeLastIndex * oldColumn.componentSize);
                    componentInfo->destructor(oldColumn.data + oldComponentIndex * oldColumn.componentSize);
                }

                // update lookup tables
                oldArchetype->entityIDToIndex[oldArchetype->indexToEntityID.back()] = oldArchetype->entityIDToIndex[entityID];
                oldArchetype->indexToEntityID[oldArchetype->entityIDToIndex[entityID]] = oldArchetype->indexToEntityID.back();
                oldArchetype->entityIDToIndex.erase(entityID);
                oldArchetype->indexToEntityID.pop_back();
                --oldArchetype->entityCount;

                newArchetype->entityIDToIndex.emplace(entityID, newComponentIndex);
                newArchetype->indexToEntityID.emplace_back(entityID);
                ++newArchetype->entityCount;

                // update record
                GetEntityRecord(entityID).archetype = newArchetype;
            }

            entityIDs.clear();
        }
    }

    void Entity::FlushFutures()
    {
        FlushFutureRemovedComponents();

        for (EntityID locToDeleteEntity : locToEraseEntities)
        {
            EraseEntity(locToDeleteEntity);
        }
        locToEraseEntities.clear();

        for (EntityID locToEmplaceEntity : locToEmplaceEntities)
        {
            EmplaceEntity(locToEmplaceEntity);
        }
        locToEmplaceEntities.clear();

        FlushFutureComponents();
    }

    void Entity::CreateFutureArray(const ComponentID aID, const size_t aSize)
    {
        static constexpr size_t START_FUTURE_ARRAY_CAPACITY = 2;
        std::byte* dataPointer = new std::byte[START_FUTURE_ARRAY_CAPACITY * aSize];
        EntityID* entityIDsPointer = new EntityID[START_FUTURE_ARRAY_CAPACITY];
#ifdef _DEBUG
        std::memset(dataPointer, 0, START_FUTURE_ARRAY_CAPACITY * aSize);
        std::memset(entityIDsPointer, 0, START_FUTURE_ARRAY_CAPACITY * sizeof(EntityID));
#endif
        ourFutureComponents.emplace(aID, Internal::FutureComponentsArray{ dataPointer, aSize, entityIDsPointer, 0, START_FUTURE_ARRAY_CAPACITY });
    }
}
