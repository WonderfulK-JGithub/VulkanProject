#pragma once
#include <vector>

#include "Events.h"

#include <array>
#include <string>

#include "ID/IdTypes.h"

namespace Wonder
{
	struct PrefabAssetUpdateEvent
	{
		PrefabId prefabId;
	};
}

DECLARE_EVENT_TYPE(Wonder::PrefabAssetUpdateEvent);