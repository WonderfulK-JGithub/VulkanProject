#pragma once
#include <vector>

#include "Events.h"

#include <array>
#include <string>
#include <tge/stringRegistry/StringRegistry.h>

#include "IdTypes.h"

namespace Goose
{
	class PropertySetTypeBase;

	enum class EventFlags : uint8_t
	{
		Respawn,
		Pickup
	};

	struct PrefabUpdateInfo
	{
		std::vector<const PropertySetTypeBase*> removedPropertySets;
		PrefabInstanceID prefabID = static_cast<PrefabInstanceID>(-1);
	};

	struct PrefabUpdateEvent
	{
		PrefabUpdateInfo* eventInfo = nullptr;
	};

	struct AssetBrowserFocusEvent
	{
		std::string newFocusPath;
	};

	enum class AssetType
	{
		ParticleSystem,
		AnimationController
	};

	struct AssetUpdateEvent
	{
		AssetType type;
		UniversalInstanceID id = NULL_INSTANCE;
	};

	struct PlayerUnlockInfo
	{
		int abilityIndex;
	};

	struct PlayerHealthUpdateEvent
	{
		int currentPlayerHealth = 0;
	};

	struct PlayerJuiceUpdateEvent
	{
		float currentPlayerJuice = 0;
	};

	struct PlayerAbilityCooldownEvent
	{
		std::array<float, 6> abilityCooldowns;
	};

	struct PlayerUnlockEvent
	{
		PlayerUnlockInfo* eventInfo = nullptr;
	};

	struct ScriptPostEvent
	{
		Tga::StringId name;
		SceneInstanceID id = NULL_INSTANCE;
	};

	struct SceneLoadEvent
	{
		
	};
}

DECLARE_EVENT_TYPE(Goose::EventFlags);
DECLARE_EVENT_TYPE(Goose::PrefabUpdateEvent);
DECLARE_EVENT_TYPE(Goose::PlayerUnlockEvent);
DECLARE_EVENT_TYPE(Goose::AssetBrowserFocusEvent);
DECLARE_EVENT_TYPE(Goose::AssetUpdateEvent);
DECLARE_EVENT_TYPE(Goose::PlayerHealthUpdateEvent);
DECLARE_EVENT_TYPE(Goose::PlayerJuiceUpdateEvent);
DECLARE_EVENT_TYPE(Goose::PlayerAbilityCooldownEvent);
DECLARE_EVENT_TYPE(Goose::ScriptPostEvent);
DECLARE_EVENT_TYPE(Goose::SceneLoadEvent);