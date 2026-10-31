#include "ECSInstance.h"

#include "Entity.h"

#include <stack>

namespace
{
	std::stack<ECS::ECSInstance*> locECSStack{};
}

ECS::ECSScope::ECSScope(ECSInstance* aECS)
{
	PushECS(aECS);
}

ECS::ECSScope::~ECSScope()
{
	PopECS();
}

void ECS::PushECS(ECSInstance* aECS)
{
	locECSStack.push(aECS);
	globalCurrentECS = aECS;
}

void ECS::PopECS()
{
	Entity::FlushFutures();
	locECSStack.pop();
	globalCurrentECS = locECSStack.empty() ? nullptr : locECSStack.top();
}

int ECS::GetECSStackSize()
{
	return static_cast<int>(locECSStack.size());
}
