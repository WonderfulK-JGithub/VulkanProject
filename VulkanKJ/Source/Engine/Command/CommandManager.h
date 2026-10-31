#pragma once
#include <memory>
#include <vector>

#include "Command.h"

#include <functional>

namespace Wonder
{
	class CommandManager
	{
	public:
		void SetFunctions(const std::function<void(const std::shared_ptr<Command>& aCommand)>& aOnUndo,
			const std::function<void(const std::shared_ptr<Command>& aCommand)>& aOnAddRedo,
			const std::function<void()>& aOnRemovedCommands);

		void AddAndExecute(const std::shared_ptr<Command>& aCommand);
		void Add(const std::shared_ptr<Command>& aCommand);
		void TryMergeTopCommands();

		void Undo();
		void Redo();

		void SetSaveCommandCount();
		bool IsSameAsLastSave() const;
		
		void ClearQueue();
		
		std::vector<std::shared_ptr<Command>>& AccessCommands();

	private:
		std::vector<std::shared_ptr<Command>> myCommands;
		std::function<void(const std::shared_ptr<Command>& aCommand)> myOnAddRedo;
		std::function<void(const std::shared_ptr<Command>& aCommand)> myOnUndo;
		std::function<void()> myOnRemovedCommands;
		int myCommandSinceLastSave = -1;
		int myCurrentCommand       = -1;
	};
}


