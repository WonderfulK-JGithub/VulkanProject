#include "DebugUtilities.h"

#include <glm/gtc/constants.hpp>

#include "imgui.h"
#include "Math/MathConstants.h"
#include "Vulkan/VkMesh.h"

bool localTabItemOpen = false;
bool localTabBarOpen = false;
bool localDebugWindowOpen = false;

namespace DebugUtilities
{
	bool BeginDebugTabItem(const char* aLabel, bool* aOpen, const int aTabItemFlags)
	{
		if (ImGui::Begin("DebugWindow", nullptr, ImGuiWindowFlags_NoDocking))
		{
			localDebugWindowOpen = true;
			if (ImGui::BeginTabBar("DebugTabs"))
			{
				localTabBarOpen = true;
				if (ImGui::BeginTabItem(aLabel, aOpen,aTabItemFlags))
				{
					localTabItemOpen = true;
					return true;
				}
			}
		}

		return false;
	}

	void EndDebugTabItem()
	{
		if (localTabItemOpen)
		{
			localTabItemOpen = false;
			ImGui::EndTabItem();
		}
		if (localTabBarOpen)
		{
			localTabBarOpen = false;
			ImGui::EndTabBar();
		}

		localDebugWindowOpen = false;
		ImGui::End();
	}

	void AddSphereLines(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const float aRadius, const glm::vec4 aLineColor,const int aCircleLineCount)
	{
		const float circleIncrement = glm::tau<float>() / static_cast<float>(aCircleLineCount);

		// z plane
		{
			glm::vec3 lastPos = glm::vec3{aRadius, 0.f, 0.f} + aPosition;
			float angle           = circleIncrement;
			for (int i = 0; i < aCircleLineCount; i++)
			{
				glm::vec3 newPos = glm::vec3{std::cosf(angle) * aRadius, std::sinf(angle) * aRadius, 0.f} + aPosition;
				aLines.emplace_back(lastPos,aLineColor);
				aLines.emplace_back(newPos,aLineColor);
				lastPos = newPos;
				angle += circleIncrement;
			}
		}

		// y plane
		{
			glm::vec3 lastPos = glm::vec3{aRadius, 0.f, 0.f} + aPosition;
			float angle           = circleIncrement;
			for (int i = 0; i < aCircleLineCount; i++)
			{
				glm::vec3 newPos = glm::vec3{std::cosf(angle) * aRadius, 0.f, std::sinf(angle) * aRadius} + aPosition;
				aLines.emplace_back(lastPos,aLineColor);
				aLines.emplace_back(newPos,aLineColor);
				lastPos = newPos;
				angle += circleIncrement;
			}
		}

		// x plane
		{
			glm::vec3 lastPos = glm::vec3{0.f, 0.f, aRadius} + aPosition;
			float angle           = circleIncrement;
			for (int i = 0; i < aCircleLineCount; i++)
			{
				glm::vec3 newPos = glm::vec3{0.f, std::sinf(angle) * aRadius, std::cosf(angle) * aRadius} + aPosition;
				aLines.emplace_back(lastPos,aLineColor);
				aLines.emplace_back(newPos,aLineColor);
				lastPos = newPos;
				angle += circleIncrement;
			}
		}
	}

	void AddSphereLinesArc(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, float aRadius, float aArc,
		glm::vec4 aLineColor)
	{
		glm::vec4 otherColor = aLineColor;
		if (aArc < glm::tau<float>())
		{
			otherColor = otherColor * 0.8f;
		}
		
		glm::mat4x4 referenceTransform;
		referenceTransform[3] = glm::vec4(aPosition,1.f);
		
		//moving halfCircle
		referenceTransform[0] = glm::vec4(aRotation * Wonder::DOWN_VEC_3,0.f);
		referenceTransform[1] = glm::vec4(glm::angleAxis(aArc,aRotation * Wonder::DOWN_VEC_3) * (aRotation * Wonder::RIGHT_VEC_3),0.f);
		AddCircleLinesArc(aLines, referenceTransform, aRadius, glm::pi<float>(), otherColor);

		// y plane
		referenceTransform[0] = glm::vec4(aRotation * Wonder::RIGHT_VEC_3,0.f);
		referenceTransform[1] = glm::vec4(aRotation * Wonder::FORWARD_VEC_3,0.f);
		AddCircleLinesArc(aLines, referenceTransform, aRadius, aArc, aLineColor);

		// forward z plane
		referenceTransform[0] = glm::vec4(aRotation * Wonder::DOWN_VEC_3,0.f);
		referenceTransform[1] = glm::vec4(aRotation * Wonder::RIGHT_VEC_3,0.f);
		AddCircleLinesArc(aLines, referenceTransform, aRadius, glm::pi<float>(), otherColor);

		
		if (aArc > glm::half_pi<float>())
		{
			referenceTransform[0] = glm::vec4(aRotation * Wonder::DOWN_VEC_3,0.f);
			referenceTransform[1] = glm::vec4(aRotation * Wonder::FORWARD_VEC_3,0.f);
			AddCircleLinesArc(aLines, referenceTransform, aRadius, glm::pi<float>(), aLineColor);
		}

		if (aArc > glm::pi<float>())
		{
			referenceTransform[0] = glm::vec4(aRotation * Wonder::UP_VEC_3,0.f);
			referenceTransform[1] = glm::vec4(aRotation * Wonder::RIGHT_VEC_3,0.f);
			AddCircleLinesArc(aLines, referenceTransform, aRadius, glm::pi<float>(), aLineColor);
		}

		if (aArc > glm::pi<float>() * 1.5f)
		{
			referenceTransform[0] = glm::vec4(aRotation * Wonder::DOWN_VEC_3,0.f);
			referenceTransform[1] = glm::vec4(aRotation * Wonder::BACK_VEC_3,0.f);
			AddCircleLinesArc(aLines, referenceTransform, aRadius, glm::pi<float>(), aLineColor);
			
		}

		if (aArc < glm::tau<float>())
		{
			aLines.emplace_back(aPosition - aRotation * Wonder::DOWN_VEC_3 * aRadius,aLineColor);
			aLines.emplace_back(aPosition - aRotation * Wonder::UP_VEC_3 * aRadius,aLineColor);
		}
		
	}

	void AddCylinderLines(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::vec3& aUp, float aRadius, float aHeight,
		const glm::vec4& aColor)
	{
		const float halfHeight = aHeight * 0.5f;

		
		const glm::vec3 temp = glm::normalize(glm::cross(aUp,Wonder::FORWARD_VEC_3));
		const glm::vec3 forward = glm::normalize(glm::cross(aUp,temp));
		const glm::vec3 right = glm::cross(aUp,forward);

		glm::mat4x4 transform;
		transform[2] = glm::vec4(aUp,0.f);
		transform[1] = glm::vec4(forward,0.f);
		transform[0] = glm::vec4(right,0.f);

		const glm::vec3 cylinderStart = aPosition - aUp * halfHeight;
		const glm::vec3 cylinderEnd = aPosition + aUp * halfHeight;

		transform[3] = glm::vec4(cylinderStart,1.f);
		AddCircleLines(aLines, transform, aRadius, aColor);
		transform[3] = glm::vec4(cylinderEnd,1.f);
		AddCircleLines(aLines, transform, aRadius, aColor);

		const glm::vec3 offsets[]
		{
			right,
			right * -1.f,
			forward,
			forward * -1.f
		};

		for (const glm::vec3& offset : offsets)
		{
			aLines.emplace_back(cylinderStart + offset * aRadius, aColor);
			aLines.emplace_back(cylinderEnd + offset * aRadius, aColor);
		}
	}

	void AddConeLinesArc(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, float aRadius, float aHeight,
		float aArc, glm::vec4 aLineColor)
	{
		glm::vec4 otherColor = aLineColor;
		if (aArc < glm::tau<float>())
		{
			otherColor = otherColor * 0.8f;
		}

		glm::vec3 circleCenter    = aPosition + aRotation * Wonder::UP_VEC_3 * aHeight;
		glm::vec3 circleArchPoint = circleCenter + std::cosf(aArc) * aRadius * aRotation * Wonder::RIGHT_VEC_3 + std::sinf(aArc) * aRadius *
		                                aRotation * Wonder::FORWARD_VEC_3;
		
		aLines.emplace_back(aPosition,otherColor);
		aLines.emplace_back(circleArchPoint,otherColor);
		
		aLines.emplace_back(aPosition,otherColor);
		aLines.emplace_back(circleCenter + aRotation * Wonder::RIGHT_VEC_3 * aRadius,otherColor);

		{
			glm::mat4x4 referenceTransform;
			referenceTransform[3] = glm::vec4(circleCenter,1.f);
			referenceTransform[2] = glm::vec4(aRotation * Wonder::RIGHT_VEC_3,0.f);
			referenceTransform[2] = glm::vec4(aRotation * Wonder::FORWARD_VEC_3,0.f);

			AddCircleLinesArc(aLines, referenceTransform, aRadius, aArc, aLineColor);
		}

		if (aArc > glm::half_pi<float>())
		{
			aLines.emplace_back(aPosition,aLineColor);
			aLines.emplace_back(circleCenter + aRotation * Wonder::FORWARD_VEC_3 * aRadius,aLineColor);
		}

		if (aArc > glm::pi<float>())
		{
			aLines.emplace_back(aPosition,aLineColor);
			aLines.emplace_back(circleCenter + aRotation * Wonder::LEFT_VEC_3 * aRadius,aLineColor);
		}

		if (aArc > glm::pi<float>() * 1.5f)
		{
			aLines.emplace_back(aPosition,aLineColor);
			aLines.emplace_back(circleCenter + aRotation * Wonder::BACK_VEC_3 * aRadius,aLineColor);
		}

		if (aArc < glm::tau<float>())
		{
			aLines.emplace_back(aPosition,aLineColor);
			aLines.emplace_back(circleCenter,aLineColor);
			
			aLines.emplace_back(circleCenter,aLineColor);
			aLines.emplace_back(circleArchPoint,aLineColor);
			
			aLines.emplace_back(circleCenter,aLineColor);
			aLines.emplace_back(circleCenter + aRotation * Wonder::RIGHT_VEC_3 * aRadius,aLineColor);
		}
	}

	void AddConeLines(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, float aRadius, float aHeight, glm::vec4 aLineColor)
	{
		glm::vec4 otherColor = aLineColor;

		glm::vec3 circleCenter    = aPosition + aRotation * Wonder::BACK_VEC_3 * aHeight;
		
		aLines.emplace_back(aPosition,otherColor);
		aLines.emplace_back(circleCenter + aRotation * Wonder::BACK_VEC_3 * aRadius,otherColor);
		
		aLines.emplace_back(aPosition,otherColor);
		aLines.emplace_back(circleCenter + aRotation * Wonder::LEFT_VEC_3 * aRadius,otherColor);
		
		aLines.emplace_back(aPosition,otherColor);
		aLines.emplace_back(circleCenter + aRotation * Wonder::UP_VEC_3 * aRadius,otherColor);
		
		aLines.emplace_back(aPosition,otherColor);
		aLines.emplace_back(circleCenter + aRotation * Wonder::DOWN_VEC_3 * aRadius,otherColor);

		{
			glm::mat4x4 referenceTransform;
			referenceTransform[3] = glm::vec4(circleCenter,1.f);
			referenceTransform[0] = glm::vec4(aRotation * Wonder::RIGHT_VEC_3,0.f);
			referenceTransform[1] = glm::vec4(aRotation * Wonder::UP_VEC_3,0.f);

			AddCircleLines(aLines, referenceTransform, aRadius, aLineColor);
		}
	}

	void AddCircleLinesArc(std::vector<Vulkan::LineVertex>& aLines, const glm::mat4x4& aTransform, const float aRadius,
	                       const float aArc, const glm::vec4 aLineColor)
	{
		constexpr int circleLineCount = 75;

		const float circleIncrement = aArc / static_cast<float>(circleLineCount);
		glm::vec3 lastPos       = xyz(aTransform[3]) + xyz(aTransform[2]) * aRadius;
		float angle                 = circleIncrement;
		for (int i = 0; i < circleLineCount; i++)
		{
			glm::vec3 newPos = xyz(aTransform[3]);
			newPos += std::cosf(angle) * aRadius * xyz(aTransform[2]);
			newPos += std::sinf(angle) * aRadius * xyz(aTransform[1]);

			aLines.emplace_back(lastPos,aLineColor);
			aLines.emplace_back(newPos,aLineColor);
			lastPos = newPos;
			angle += circleIncrement;
		}
	}

	void AddCircleLines(std::vector<Vulkan::LineVertex>& aLines, const glm::mat4x4& aTransform, float aRadius, glm::vec4 aLineColor)
	{
		constexpr int circleLineCount = 75;
		constexpr float circleIncrement = glm::tau<float>() / static_cast<float>(circleLineCount);
		
		glm::vec3 lastPos       = xyz(aTransform[3]) + xyz(aTransform[2]) * aRadius;
		float angle                 = circleIncrement;
		
		for (int i = 0; i < circleLineCount; i++)
		{
			glm::vec3 newPos = xyz(aTransform[3]);
			newPos += std::cosf(angle) * aRadius * xyz(aTransform[2]);
			newPos += std::sinf(angle) * aRadius * xyz(aTransform[1]);

			aLines.emplace_back(lastPos,aLineColor);
			aLines.emplace_back(newPos,aLineColor);
			lastPos = newPos;
			angle += circleIncrement;
		}
	}

	void AddBoxLines(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3& aPosition, const glm::quat& aRotation, const glm::vec3& aSize,
	                 const glm::vec4 aLineColor)
	{
		const glm::vec3 halfSize = aSize * 0.5f;

		glm::vec3 vertices[8] = {
			{-halfSize.x,  halfSize.y, -halfSize.z },
			{-halfSize.x,  halfSize.y,  halfSize.z },
			{ halfSize.x,  halfSize.y,  halfSize.z },
			{ halfSize.x,  halfSize.y, -halfSize.z },
			{-halfSize.x, -halfSize.y, -halfSize.z },
			{-halfSize.x, -halfSize.y,  halfSize.z },
			{ halfSize.x, -halfSize.y,  halfSize.z },
			{ halfSize.x, -halfSize.y, -halfSize.z }
		};

		for (int i = 0; i < 8; ++i)
		{
			vertices[i] = vertices[i].x * aRotation * Wonder::RIGHT_VEC_3 + vertices[i].y * aRotation * Wonder::UP_VEC_3 
			+ vertices[i].z * aRotation * Wonder::FORWARD_VEC_3;
			vertices[i] += aPosition;
		}

		for (int i = 0; i < 4; ++i)
		{
			aLines.emplace_back(vertices[i], aLineColor);
			aLines.emplace_back(vertices[(i + 1) % 4], aLineColor);
			
			aLines.emplace_back(vertices[i], aLineColor);
			aLines.emplace_back(vertices[(i + 1) % 4 + 4], aLineColor);
			
			aLines.emplace_back(vertices[i], aLineColor);
			aLines.emplace_back(vertices[(i + 4)], aLineColor);
		}
	}

	void AddVector(std::vector<Vulkan::LineVertex>& aLines, const glm::vec3 aStartPos, const glm::vec3 aVector, const glm::vec3 aCameraPos,
		const glm::vec4 aLineColor, const float aSideLength)
	{
		const glm::vec3 vectorDirection = glm::normalize(aVector);

		const glm::vec3 endPoint = aStartPos + aVector;
		aLines.emplace_back(aStartPos, aLineColor);
		aLines.emplace_back(endPoint, aLineColor);

		const glm::vec3 rotationAxis = glm::normalize(aCameraPos - endPoint);
		
		aLines.emplace_back(endPoint, aLineColor);
		aLines.emplace_back(endPoint + glm::angleAxis(glm::radians(210.f),rotationAxis) * vectorDirection * aSideLength, aLineColor);
		
		aLines.emplace_back(endPoint, aLineColor);
		aLines.emplace_back(endPoint + glm::angleAxis(glm::radians(150.f),rotationAxis) * vectorDirection * aSideLength, aLineColor);
	}
}
