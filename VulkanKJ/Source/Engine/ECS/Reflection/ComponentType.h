#pragma once
#include "ComponentReflectionUtil.h"
#include "ECS/Entity.h"

#include <cstdint>
#include <span>

#include "Util/StringRegistry.h"

namespace Wonder
{
	using ComponentID = uint32_t;

	inline constexpr ComponentID INVALID_COMPONENT_ID = (std::numeric_limits<ComponentID>::max)();

	class ComponentTypeBase
	{
	public:
		virtual ~ComponentTypeBase() = default;
		ComponentTypeBase(const char* aName, void* aDefaultValue, uint32_t aByteSize);

		uint32_t GetByteSize() const;
		const char* GetName() const;
		Tga::StringId GetNameStringID() const;

		virtual void EmplaceNewInAddress(char* aAddress) const = 0;
		virtual void CopyToAddress(char* aAddress, const char* aCopyData) const = 0;
		virtual void DestructorInAddress(char* aAddress) const = 0;

		virtual void ReflectComponent(char* aData) const = 0;

		virtual void SetComponentData(ECS::Entity aEntity, const char* aData) const = 0;
		virtual void GetFromECS(ECS::Entity aEntity,char* aData) const = 0;
		virtual void OnGameComponentCreate(ECS::Entity aEntity, const char* aData) const = 0;

		static void FinalizeTypeRegistry();

		virtual size_t GetHashCode() const = 0;

	protected:
		uint32_t myID = INVALID_COMPONENT_ID;
		uint32_t myByteSize = 0;
		Tga::StringId myNameStringID;
		const char* myName = nullptr;
		void* myDefaultValue = nullptr;
	};

	template<typename Type>
	class ComponentType final : public ComponentTypeBase // fuck arv
	{
	public:
		ComponentType(const char* aName,
			void* aDefaultValue)
			:
			ComponentTypeBase(aName, aDefaultValue, sizeof(Type))
		{
		}

		void OnGameComponentCreate(ECS::Entity aEntity, const char* aData) const override;

		void EmplaceNewInAddress(char* aAddress) const override;
		void CopyToAddress(char* aAddress, const char* aCopyData) const override;
		void DestructorInAddress(char* aAddress) const override;
		void ReflectComponent(char* aData) const override;
		void SetComponentData(ECS::Entity aEntity, const char* aData) const override;
		size_t GetHashCode() const override;
		void GetFromECS(ECS::Entity aEntity,char* aData) const override;
	};

	template<typename Type>
	void ComponentType<Type>::OnGameComponentCreate(ECS::Entity aEntity,const char* aData) const
	{
		aEntity.AddComponent<Type>(*reinterpret_cast<const Type*>(aData));
		ECS::OnComponentDataSet(*aEntity.GetComponent<Type>());
		ECS::OnComponentDataSet(aEntity, *aEntity.GetComponent<Type>());
	}

	template<typename Type>
	void ComponentType<Type>::EmplaceNewInAddress(char* aAddress) const
	{
		if (myDefaultValue == nullptr)
		{
			new(aAddress) Type();
		}
		else
		{
			new(aAddress) Type(*static_cast<const Type*>(myDefaultValue));
		}
	}

	template<typename Type>
	void ComponentType<Type>::CopyToAddress(char* aAddress, const char* aCopyData) const
	{
		new(aAddress) Type(*reinterpret_cast<const Type*>(aCopyData));
	}

	template<typename Type>
	void ComponentType<Type>::DestructorInAddress(char* aAddress) const
	{
		reinterpret_cast<Type*>(aAddress)->~Type();
	}

	template<typename Type>
	void ComponentType<Type>::ReflectComponent(char* aData) const
	{
		Wonder::Reflection::globalPropertyIndex = 0u;
		ECS::OnReflectComponent(*reinterpret_cast<Type*>(aData));
	}

	template<typename Type>
	void ComponentType<Type>::SetComponentData(ECS::Entity aEntity, const char* aData) const
	{
		*aEntity.GetComponent<Type>() = *reinterpret_cast<const Type*>(aData);
		ECS::OnComponentDataSet(*aEntity.GetComponent<Type>());
		ECS::OnComponentDataSet(aEntity, *aEntity.GetComponent<Type>());
	}

	template <typename Type>
	size_t ComponentType<Type>::GetHashCode() const
	{
		return typeid(Type).hash_code();
	}

	template<typename Type>
	void ComponentType<Type>::GetFromECS(ECS::Entity aEntity,char* aData) const
	{
		Type* ecsData = aEntity.GetComponent<Type>();
		if (ecsData != nullptr)
		{
			*reinterpret_cast<Type*>(aData) = *ecsData;
		}
	}

	template<typename Component>
	struct ComponentTypeDefinition
	{
	};

	//static void LoadPropertySetToECS(int aPropertySetIndex, char* aPropertySetData);

#define DECLARE_COMPONENT_TYPE(T) template<> \
struct ComponentTypeDefinition<T> \
{ \
static ComponentType<T> Type;\
};\

#define IMPLEMENT_COMPONENT_TYPE(T, NAME,DEFAULT_VALUE) \
ComponentType<T> ComponentTypeDefinition<T>::Type(NAME,DEFAULT_VALUE);\


	const Wonder::ComponentTypeBase* GetComponentType(uint64_t aHashCode);
	const Wonder::ComponentTypeBase* GetComponentType(Tga::StringId aString);
	std::span<Tga::StringId> GetAllComponentNames();
}
