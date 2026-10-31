#pragma once

#include "../Command.h"

#include <memory>

namespace Wonder
{
	class MultiCommand final : public Command
	{
	public:
		void AddCommand(const std::shared_ptr<Command>& aCommand);

		void Redo() override;
		void Undo() override;

		std::vector<IdType> GetIds() override;

	private:
		std::vector<std::shared_ptr<Command>> myCommands;
	};
}
