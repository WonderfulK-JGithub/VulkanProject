#pragma once

#include <set>
#include <unordered_map>
#include <vector>

#include "Defines.h"
#include "Util/Logging/Logging.h"

namespace ECS
{
	class Entity;

	struct ComponentArray
	{
		std::byte* data;
		size_t componentSize;
	};
	
	struct Archetype
	{
		ArchetypeID archetypeID;
		std::set<ComponentID> componentSet;

		size_t entityCount;
		size_t capacity;
		std::unordered_map<ComponentID, ComponentArray> componentArrays;
		
		std::unordered_map<EntityID, size_t> entityIDToIndex;
		std::vector<EntityID> indexToEntityID;
		
		std::unordered_map<ComponentID, Archetype*> archetypeEdgeUp;
		std::unordered_map<ComponentID, Archetype*> archetypeEdgeDown;

		~Archetype()
		{
			LOG_DEBUG("Archetype destroyed, id: %d",archetypeID);
		}
	};
	
	class ArchetypeManager
	{
	public:
		Archetype& GetArchetype(const std::set<ComponentID>& aComponentIDs)
		{
			for (Archetype* archetype : myArchetypes)
			{
				if (archetype->componentSet == aComponentIDs)
				{
					return *archetype;
				}
			}
			
			return CreateArchetype(aComponentIDs);
		}
		
		Archetype& GetArchetype(ArchetypeID aArchetypeID)
		{
			return *myArchetypes.at(aArchetypeID);
		}

		Archetype& CreateArchetype(const std::set<ComponentID>& aComponents);

		void IncreaseArchetypeCapacity(Archetype* aArchetype);

		void RemoveEntityFromArchetype(Entity aEntity);

		std::vector<Archetype*>& GetArchetypes();

		bool ArchetypeHasComponents(const Archetype* aArchetype, const std::vector<ComponentID>& aComponents);
		
		unsigned GetLastCreationTime() const { return myLastCreationTime; }

	private:
		std::vector<Archetype*> myArchetypes;
		unsigned myLastCreationTime = 0;

		ArchetypeID myNextArchetypeID{0};
	};
}

