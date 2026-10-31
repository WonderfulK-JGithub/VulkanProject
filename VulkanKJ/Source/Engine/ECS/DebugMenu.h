#pragma once
#include "Entity.h"
#include "View.hpp"
#include "imgui/imgui.h"

namespace ECS::DebugMenu
{
	void Display();

	inline void Display()
	{
		if (ImGui::Begin("ECS Debug Menu"))
		{
			ImGui::TextUnformatted(("Entity count: " + std::to_string(ECS::GetEntityCount())).data());

			std::unordered_map<ECS::ComponentID, ECS::ComponentInfo> componentIndex = ECS::GetComponentIndex();
			static std::unordered_map<ECS::ComponentID, bool> isSelected;

			ImGui::TextUnformatted("Filters");
			for (std::pair<const unsigned, ComponentInfo> index : componentIndex)
			{
				ImGui::Checkbox(index.second.name.data(), &isSelected[index.first]);
			}

			std::vector<ECS::ComponentID> componentIDs{};
			for (std::pair<const unsigned, bool> selected : isSelected)
			{
				if (selected.second)
				{
					componentIDs.emplace_back(selected.first);
				}
			}

			size_t selectedEntityCount{0};
			if (componentIDs.empty())
			{
				selectedEntityCount = ArchetypeManager::GetArchetype(std::set<ComponentID>{}).entityCount;
			}
			else
			{
				for (ECS::Archetype* archetype : ECS::ArchetypeManager::GetArchetypes())
				{
					if (ECS::ArchetypeManager::ArchetypeHasComponents(archetype, componentIDs))
					{
						selectedEntityCount += archetype->entityCount;
					}
				}
			}
			
			ImGui::TextUnformatted(("Selected entity count: " + std::to_string(selectedEntityCount)).data());
		}
		ImGui::End();
	}
}
