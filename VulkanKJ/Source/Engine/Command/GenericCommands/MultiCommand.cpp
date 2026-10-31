#include "MultiCommand.h"

void Wonder::MultiCommand::AddCommand(const std::shared_ptr<Command>& aCommand)
{
	myCommands.push_back(aCommand);
}

void Wonder::MultiCommand::Redo()
{
	for (int i = 0; i < static_cast<int>(myCommands.size()); ++i)
	{
		myCommands[i]->Redo();
	}
}

void Wonder::MultiCommand::Undo()
{
	for (int i = static_cast<int>(myCommands.size()) - 1; i >= 0; --i)
	{
		myCommands[i]->Undo();
	}
}

std::vector<Wonder::IdType> Wonder::MultiCommand::GetIds()
{
	std::vector<IdType> ids;

	for (auto& command : myCommands)
	{
		std::vector<IdType> commandIds = command->GetIds();
		ids.insert(ids.end(), commandIds.begin(), commandIds.end());
	}

	return ids;
}
