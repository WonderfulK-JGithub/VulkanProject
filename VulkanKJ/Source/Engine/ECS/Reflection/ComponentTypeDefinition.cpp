#include "ComponentType.h"
#include "ECS/Components/Rendering/ModelComponent.h"

// Declaration
namespace Wonder
{
	// DECLARE_COMPONENT_TYPE(SpriteComponent)
	DECLARE_COMPONENT_TYPE(ModelComponent)
	// DECLARE_COMPONENT_TYPE(AnimationModelComponent)
}

//Definition
namespace Wonder
{
	// Same workaround as the old editor
	void EnsureComponentTypesAreLoaded() // NOLINT(misc-use-internal-linkage)
	{
		LOG_INFO("Component types loaded!");
	}

	IMPLEMENT_COMPONENT_TYPE(ModelComponent, "Model", nullptr)
	// IMPLEMENT_COMPONENT_TYPE(AnimationModelComponent, "Animation Model", nullptr)
	// IMPLEMENT_COMPONENT_TYPE(SpriteComponent,"Sprite Component",nullptr)
}
