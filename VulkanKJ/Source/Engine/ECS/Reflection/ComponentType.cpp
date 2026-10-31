#include "ComponentType.h"

constexpr size_t MAX_TYPE_COUNT = 512;

static uint32_t locTypeCount;
static Wonder::ComponentTypeBase* locType[MAX_TYPE_COUNT];
static std::unordered_map<Tga::StringId, const Wonder::ComponentTypeBase*> locStringIdToComponentType;
static std::vector<Tga::StringId> locSortedComponentNames;

Wonder::ComponentTypeBase::ComponentTypeBase(const char* aName, void* aDefaultValue,const uint32_t aByteSize)
{
	myName = aName;
	myDefaultValue = aDefaultValue;
	myByteSize = aByteSize;
	locType[locTypeCount] = this;
	locTypeCount++;
}

uint32_t Wonder::ComponentTypeBase::GetByteSize() const
{
	return myByteSize;
}

const char* Wonder::ComponentTypeBase::GetName() const
{
	return myName;
}

Tga::StringId Wonder::ComponentTypeBase::GetNameStringID() const
{
	return myNameStringID;
}

void Wonder::ComponentTypeBase::FinalizeTypeRegistry()
{
	locSortedComponentNames.reserve(locTypeCount);

	for (int i = 0; i < static_cast<int>(locTypeCount); i++)
	{
		Tga::StringId stringId = Tga::StringRegistry::RegisterOrGetString(locType[i]->myName);
		locType[i]->myNameStringID = stringId;
		locStringIdToComponentType[stringId] = locType[i];

		locSortedComponentNames.push_back(stringId);
	}

	//std::sort(locSortedComponentNames.begin(), locSortedComponentNames.end()); // NOLINT(modernize-use-ranges)
}

const Wonder::ComponentTypeBase* Wonder::GetComponentType(uint64_t aHashCode)
{
	for (uint32_t i = 0; i < locTypeCount; ++i)
	{
		if (locType[i]->GetHashCode() == aHashCode)
		{
			return locType[i];
		}
	}

	return nullptr;
}

const Wonder::ComponentTypeBase* Wonder::GetComponentType(const Tga::StringId aString)
{
	if (locStringIdToComponentType.find(aString) == locStringIdToComponentType.end())
	{
		return nullptr;
	}
	
	return locStringIdToComponentType.at(aString);
}

std::span<Tga::StringId> Wonder::GetAllComponentNames()
{
	return locSortedComponentNames;
}
