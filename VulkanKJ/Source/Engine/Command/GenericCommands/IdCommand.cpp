#include "IdCommand.h"

Wonder::IdCommand::IdCommand(void* aObject, IdType aId, const IdFunctionPointer aUndoFunction,
	const IdFunctionPointer aRedoFunction) :
	myObject(aObject),
	myId(aId),
	myUndoFunction(aUndoFunction),
	myRedoFunction(aRedoFunction)
{
}

void Wonder::IdCommand::Redo()
{
	(myRedoFunction)(myObject, myId);
}

void Wonder::IdCommand::Undo()
{
	(myUndoFunction)(myObject, myId);
}

std::vector<Wonder::IdType> Wonder::IdCommand::GetIds()
{
	return { myId };
}
