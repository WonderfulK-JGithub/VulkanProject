#pragma once
#include <Id/IdTypes.h>
#include <vector>

namespace Wonder
{
	class CommandManager;
}

class Command
{
public:
	virtual ~Command() = default;

	virtual void Execute()
	{
		Redo();
	}

	virtual std::vector<Wonder::IdType> GetIds()
	{
		return {};
	}

	virtual void Redo() = 0;
	virtual void Undo() = 0;
	
	virtual bool ShouldConsumeNextCommand([[maybe_unused]] const Command* aNextCommand) { return false; } 
	virtual void ConsumeNextCommand(Command* aNextCommand) { __noop(aNextCommand); } 
	
	virtual bool OnAdd(Wonder::CommandManager* aCommandManager) { __noop(aCommandManager); return false; } 
};
