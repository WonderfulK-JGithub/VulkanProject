#include "ComponentData.h"

#include "ComponentType.h"

Wonder::ComponentData::ComponentData(const ComponentData& aOther)
{
	if (aOther.myType == nullptr)
	{
		return;
	}

	if (aOther.myType != myType)
	{
		if (myType != nullptr)
		{
			myType->DestructorInAddress(myData.data());
		}
		myType = aOther.myType;
		myData.resize(aOther.myData.size());
	}
	myType->CopyToAddress(myData.data(), aOther.myData.data());
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;
}

Wonder::ComponentData& Wonder::ComponentData::operator=(const ComponentData& aOther)
{
	if (aOther.myType == nullptr)
	{
		return *this;
	}

	if (aOther.myType != myType)
	{
		if (myType != nullptr)
		{
			myType->DestructorInAddress(myData.data());
		}
		myType = aOther.myType;
		myData.resize(aOther.myData.size());
	}
	myType->CopyToAddress(myData.data(), aOther.myData.data());
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;

	return *this;
}

Wonder::ComponentData::ComponentData(ComponentData&& aOther) noexcept
{
	if (myType != nullptr)
	{
		myType->DestructorInAddress(myData.data());
	}

	myType = aOther.myType;
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;

	aOther.myType = nullptr;
	aOther.myAllowOverridesFlag = 0;

	myData = std::move(aOther.myData);
}

Wonder::ComponentData& Wonder::ComponentData::operator=(ComponentData&& aOther) noexcept
{
	if (myType != nullptr)
	{
		myType->DestructorInAddress(myData.data());
	}

	myType = aOther.myType;
	myAllowOverridesFlag = aOther.myAllowOverridesFlag;

	aOther.myType = nullptr;
	aOther.myAllowOverridesFlag = 0;

	myData = std::move(aOther.myData);

	return *this;
}

Wonder::ComponentData::~ComponentData()
{
	if (myType != nullptr)
	{
		myType->DestructorInAddress(myData.data());
	}
}

Wonder::ComponentData::ComponentData(const ComponentTypeBase* aType)
{
	myType = aType;
	myData.resize(myType->GetByteSize());
	myType->EmplaceNewInAddress(myData.data());
}

Wonder::ComponentData::ComponentData(const ComponentTypeBase* aType, const char* aData, const uint64_t aAllowOverrideFlag)
{
	myType = aType;
	myData.resize(myType->GetByteSize());
	myType->EmplaceNewInAddress(myData.data());
	myType->CopyToAddress(myData.data(), aData);
	myAllowOverridesFlag = aAllowOverrideFlag;
}

char* Wonder::ComponentData::AccessData()
{
	return myData.data();
}

const char* Wonder::ComponentData::AccessData() const
{
	return myData.data();
}

uint64_t& Wonder::ComponentData::AccessOverridesFlag()
{
	return myAllowOverridesFlag;
}

uint64_t Wonder::ComponentData::GetOverridesFlag() const
{
	return myAllowOverridesFlag;
}

const Wonder::ComponentTypeBase* Wonder::ComponentData::GetType() const
{
	return myType;
}
