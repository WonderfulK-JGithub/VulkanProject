#pragma once
#pragma once
#include "../Command.h"

#include <vector>

namespace Goose
{
	template<typename T>
	class UpdateListCommand final : public Command
	{
	public:
		using UpdateFunctionPointer = void (*)(void*, const std::vector<UniversalInstanceID>&, const T&);

		UpdateListCommand(void* aObject, const std::vector<UniversalInstanceID>& aIds, const T& aBeforeValue, const T& aAfterValue, UpdateFunctionPointer aFunction);

		void Redo() override;
		void Undo() override;

		std::vector<Goose::UniversalInstanceID> GetIds() override;

	private:
		T myBeforeValue;
		T myAfterValue;
		std::vector<UniversalInstanceID> myIds;
		void* myObject = nullptr;
		UpdateFunctionPointer myFunction = nullptr;
	};

	template<typename T>
	UpdateListCommand<T>::UpdateListCommand(void* aObject, const std::vector<UniversalInstanceID>& aIds, const T& aBeforeValue, const T& aAfterValue,
		const UpdateFunctionPointer aFunction) :
		myBeforeValue(aBeforeValue),
		myAfterValue(aAfterValue),
		myIds(aIds),
		myObject(aObject),
		myFunction(aFunction)
	{
	}

	template<typename T>
	void UpdateListCommand<T>::Redo()
	{
		myFunction(myObject, myIds, myAfterValue);
	}

	template<typename T>
	void UpdateListCommand<T>::Undo()
	{
		myFunction(myObject, myIds, myBeforeValue);
	}

	template<typename T>
	std::vector<Goose::UniversalInstanceID> UpdateListCommand<T>::GetIds()
	{
		return myIds;
	}
}


