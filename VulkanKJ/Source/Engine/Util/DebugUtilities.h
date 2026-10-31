#pragma once
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Vulkan
{
	struct LineVertex;
}


namespace DebugUtilities
{
	bool BeginDebugTabItem(const char* aLabel, bool* aOpen = nullptr, int aTabItemFlags = 0);
	void EndDebugTabItem();

	void AddSphereLines(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, float aRadius, glm::vec4 aLineColor = {0.f, 1.f, 1.f, 1.f}, 
		int aCircleLineCount = 75);
	
	void AddSphereLinesArc(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, float aRadius, float aArc,
		glm::vec4 aLineColor = {0.f, 1.f, 1.f, 1.f});
	void AddCylinderLines(std::vector<Vulkan::LineVertex>& aLines,const glm::vec3& aPosition, const glm::vec3& aUp, float aRadius, float aHeight, const glm::vec4& aColor);
	void AddConeLinesArc(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, float aRadius, float aHeight,
		float aArc, glm::vec4 aLineColor = {0.f, 1.f, 1.f, 1.f});

	void AddConeLines(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, float aRadius, float aHeight,
		glm::vec4 aLineColor = {0.f, 1.f, 1.f, 1.f});
	
	void AddCircleLinesArc(std::vector<Vulkan::LineVertex>& aLines, const glm::mat4x4& aTransform, float aRadius, float aArc,
	                       glm::vec4 aLineColor = {0.f, 1.f, 1.f, 1.f});

	void AddCircleLines(std::vector<Vulkan::LineVertex>& aLines, const glm::mat4x4& aTransform, float aRadius,
	                       glm::vec4 aLineColor = {0.f, 1.f, 1.f, 1.f});
	
	void AddBoxLines(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, const glm::vec3& aSize,
	                 glm::vec4 aLineColor = {0.f, 1.f, 1.f, 1.f});

	void AddVector(std::vector<Vulkan::LineVertex>& aLines, glm::vec3 aStartPos, glm::vec3 aVector, glm::vec3 aCameraPos, glm::vec4 aLineColor,
		float aSideLength);
}
