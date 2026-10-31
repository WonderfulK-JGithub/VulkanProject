#include "CommandManager.h"

void Wonder::CommandManager::SetFunctions(const std::function<void(const std::shared_ptr<Command>& aCommand)>& aOnUndo,
	const std::function<void(const std::shared_ptr<Command>& aCommand)>& aOnAddRedo, const std::function<void()>& aOnRemovedCommands)
{
	myOnUndo = aOnUndo;
	myOnAddRedo = aOnAddRedo;
	myOnRemovedCommands = aOnRemovedCommands;
}

void Wonder::CommandManager::AddAndExecute(const std::shared_ptr<Command>& aCommand)
{
	if (myCurrentCommand < static_cast<int>(myCommands.size()) - 1)
	{
		myCommands.erase(myCommands.begin() + (myCurrentCommand + 1), myCommands.end());
		if (myOnRemovedCommands != nullptr)
		{
			myOnRemovedCommands();
		}
	}

	aCommand->Redo();
	myCommands.push_back(aCommand);
	myCurrentCommand++;

	if (myOnAddRedo != nullptr)
	{
		myOnAddRedo(aCommand);
	}
	
	TryMergeTopCommands();
}

void Wonder::CommandManager::Add(const std::shared_ptr<Command>& aCommand)
{
	if (myCurrentCommand < static_cast<int>(myCommands.size()) - 1)
	{
		myCommands.erase(myCommands.begin() + (myCurrentCommand + 1), myCommands.end());
		if (myOnRemovedCommands != nullptr)
		{
			myOnRemovedCommands();
		}
	}
	myCommands.push_back(aCommand);
	myCurrentCommand++;

	if (myOnAddRedo != nullptr)
	{
		myOnAddRedo(aCommand);
	}
	//myDocument->OnAddRedo(aCommand);
	
	TryMergeTopCommands();
}

void Wonder::CommandManager::TryMergeTopCommands()
{
	if (myCurrentCommand < 1)
	{
		return;
	}
	
	std::shared_ptr<Command> previousCommand = myCommands[myCurrentCommand - 1];
	std::shared_ptr<Command> topCommand      = myCommands[myCurrentCommand];
	if (previousCommand->ShouldConsumeNextCommand(topCommand.get()))
	{
		previousCommand->ConsumeNextCommand(topCommand.get());
		myCommands.erase(myCommands.end() - 1);
		--myCurrentCommand;
	}
}

void Wonder::CommandManager::Undo()
{
	if (myCurrentCommand < 0)
	{
		return;
	}


	if (myOnUndo != nullptr)
	{
		myOnUndo(myCommands[myCurrentCommand]);
	}

	myCommands[myCurrentCommand]->Undo();
	myCurrentCommand--;

}

void Wonder::CommandManager::Redo()
{
	if (myCurrentCommand + 1 >= static_cast<int>(myCommands.size()))
	{
		return;
	}

	

	myCurrentCommand++;
	if (myOnAddRedo != nullptr)
	{
		myOnAddRedo(myCommands[myCurrentCommand]);
	}
	myCommands[myCurrentCommand]->Redo();
}

void Wonder::CommandManager::SetSaveCommandCount()
{
	myCommandSinceLastSave = myCurrentCommand;
}

bool Wonder::CommandManager::IsSameAsLastSave() const
{
	return myCommandSinceLastSave == myCurrentCommand;
}

void Wonder::CommandManager::ClearQueue()
{
	myCommands.clear();
	myCurrentCommand = 0;
}

std::vector<std::shared_ptr<Command>>& Wonder::CommandManager::AccessCommands()
{
	return myCommands;
}
