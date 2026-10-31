#pragma once

#include "../Command.h"

namespace Wonder
{
	
	class IdCommand final : public Command
	{
	public:
		using IdFunctionPointer = void (*)(void*, IdType);

		IdCommand(void* aObject, IdType aId, IdFunctionPointer aUndoFunction, IdFunctionPointer aRedoFunction);

		void Redo() override;
		void Undo() override;

		std::vector<IdType> GetIds() override;

	private:
		void* myObject                   = nullptr;
		IdType myId         = INVALID_ID_TYPE;
		IdFunctionPointer myUndoFunction = nullptr;
		IdFunctionPointer myRedoFunction = nullptr;
	};
}
