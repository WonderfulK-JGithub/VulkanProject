#pragma once

struct ComponentPack0
{
};

template<typename Component1>
struct ComponentPack1
{
	Component1 variable1;
};

template<typename Component1, typename Component2>
struct ComponentPack2
{
	Component1 variable1;
	Component2 variable2;
};

template<typename Component1, typename Component2, typename Component3>
struct ComponentPack3
{
	Component1 variable1;
	Component2 variable2;
	Component3 variable3;
};

template<typename Component1, typename Component2, typename Component3, typename Component4>
struct ComponentPack4
{
	Component1 variable1;
	Component2 variable2;
	Component3 variable3;
	Component4 variable4;
};

template<typename Component1, typename Component2, typename Component3, typename Component4, typename Component5>
struct ComponentPack5
{
	Component1 variable1;
	Component2 variable2;
	Component3 variable3;
	Component4 variable4;
	Component5 variable5;
};

template<typename Component1, typename Component2, typename Component3, typename Component4, typename Component5, typename Component6>
struct ComponentPack6
{
	Component1 variable1;
	Component2 variable2;
	Component3 variable3;
	Component4 variable4;
	Component5 variable5;
	Component6 variable6;
};

template<typename Component1, typename Component2, typename Component3, typename Component4, typename Component5, typename Component6, typename Component7>
struct ComponentPack7
{
	Component1 variable1;
	Component2 variable2;
	Component3 variable3;
	Component4 variable4;
	Component5 variable5;
	Component6 variable6;
	Component7 variable7;
};

template<typename ...Components>
static auto GetComponentPack()
{
	using EntityType = ECS::Entity;

	if constexpr (sizeof...(Components) == 0)
	{
		ComponentPack1<EntityType>* type = nullptr;
		return *type;
	}
	if constexpr (sizeof...(Components) == 1)
	{
		ComponentPack2<EntityType, Components...>* type = nullptr;
		return *type;
	}
	if constexpr (sizeof...(Components) == 2)
	{
		ComponentPack3<EntityType, Components...>* type = nullptr;
		return *type;
	}
	if constexpr (sizeof...(Components) == 3)
	{
		ComponentPack4<EntityType, Components...>* type = nullptr;
		return *type;
	}
	if constexpr (sizeof...(Components) == 4)
	{
		ComponentPack5<EntityType, Components...>* type = nullptr;
		return *type;
	}
	if constexpr (sizeof...(Components) == 5)
	{
		ComponentPack6<EntityType, Components...>* type = nullptr;
		return *type;
	}
	if constexpr (sizeof...(Components) == 6)
	{
		ComponentPack7<EntityType, Components...>* type = nullptr;
		return *type;
	}
}
