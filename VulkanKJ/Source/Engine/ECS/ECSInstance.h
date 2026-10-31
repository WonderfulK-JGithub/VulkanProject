#pragma once
#include "ArchetypeManager.h"
#include "Util/Container/MappedVector.hpp"

namespace ECS
{
	struct ECSInstance
	{
		ArchetypeManager archetypeManager;
		Goose::MappedVector<EntityID, EntityRecord> entityIndex;
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
