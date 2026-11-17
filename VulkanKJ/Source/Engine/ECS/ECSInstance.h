#pragma once
#include "ArchetypeManager.h"
#include "Util/Container/MappedVector.hpp"

namespace ECS
{
	struct ECSInstance
	{
		ArchetypeManager archetypeManager;
		Goose::MappedVector<EntityID, EntityRecord> entityRecords;
		
		std::unordered_map<EntityID,EntityParentInfo> parents;
		
		std::unordered_map<EntityReference,EntityID> referenceToIdMap;
	};

	inline ECSInstance* globalCurrentECS;

	class ECSScope
	{
	public:
		ECSScope(ECSInstance* aECS);
		~ECSScope();
	};

	void PushECS(ECSInstance* aECS);
	void PopECS();

	int GetECSStackSize();
}
