#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Vulkan
{
	struct ObjectData
	{
		glm::mat4 model;
	};

	enum class RenderStageType
	{
		Cubemap,
		Forward,
		FullscreenSwapchain,
		FullscreenOffscreen,
		ImGui,
	};
}