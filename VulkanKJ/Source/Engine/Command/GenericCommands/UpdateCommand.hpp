#pragma once
#include "../Command.h"

namespace Wonder
{
	template<typename T>
	class UpdateCommand final : public Command
	{
	public:
		using UpdateFunctionPointer = void (*)(void*, Wonder::IdType, const T&);

		UpdateCommand(void* aObject, Wonder::IdType aID, const T& aBeforeValue, const T& aAfterValue, UpdateFunctionPointer aFunction);

		void Redo() override;
		void Undo() override;

		std::vector<Wonder::IdType> GetIds() override;

	private:
		T myBeforeValue;
		T myAfterValue;
		Wonder::IdType myId         = Wonder::INVALID_ID_TYPE;
		void* myObject                   = nullptr;
		UpdateFunctionPointer myFunction = nullptr;
	};

	template<typename T>
	UpdateCommand<T>::UpdateCommand(void* aObject, const Wonder::IdType aID, const T& aBeforeValue, const T& aAfterValue,
		const UpdateFunctionPointer aFunction) :
		myBeforeValue(aBeforeValue),
		myAfterValue(aAfterValue),
		myId(aID),
		myObject(aObject),
		myFunction(aFunction)
	{
	}

	template<typename T>
	void UpdateCommand<T>::Redo()
	{
		myFunction(myObject, myId, myAfterValue);
	}

	template<typename T>
	void UpdateCommand<T>::Undo()
	{
		myFunction(myObject, myId, myBeforeValue);
	}

	template<typename T>
	std::vector<Wonder::IdType> UpdateCommand<T>::GetIds()
	{
		return {myId};
	}
}


