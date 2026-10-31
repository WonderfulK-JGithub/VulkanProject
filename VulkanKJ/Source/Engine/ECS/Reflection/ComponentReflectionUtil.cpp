#include "ComponentReflectionUtil.h"

void Wonder::Reflection::Header(const char* aText)
{
	if (globalReflectionMode == ReflectionMode::Display)
	{
		ImGuiUtil::ThickSeparator(2.f);
		ImGui::TextUnformatted(aText);
		ImGuiUtil::ThickSeparator(2.f);
	}
}
