#include "LineGrid.h"

#include <Util/DebugUtilities.h>

void Wonder::LineGrid::MakeLines(glm::vec3 aCameraPosition, int aCellCount)
{
    myLinePoints.clear();
    
    const float scaledHalfLength = static_cast<float>(aCellCount);

    constexpr glm::vec4 gridColor{0.4f, 0.4f, 0.4f, 1.0f};
    for (int i = -aCellCount; i <= aCellCount; ++i)
    {
        const float coord = static_cast<float>(i);

        if (coord != 0.f)
        {
            myLinePoints.emplace_back(glm::vec3{coord, 0.f, -scaledHalfLength},gridColor);
            myLinePoints.emplace_back(glm::vec3{coord, 0.f, scaledHalfLength},gridColor);
            
            myLinePoints.emplace_back(glm::vec3{-scaledHalfLength, 0.f, coord},gridColor);
            myLinePoints.emplace_back(glm::vec3{scaledHalfLength, 0.f, coord},gridColor);
        }
    }

    //Add Middle Lines
    {
        // x
        myLinePoints.emplace_back(glm::vec3{-scaledHalfLength, 0.f, 0.f},glm::vec4{1.f, 0.f, 0.f, 1.f});
        myLinePoints.emplace_back(glm::vec3{scaledHalfLength, 0.f, 0.f},glm::vec4{1.f, 0.f, 0.f, 1.f});
        DebugUtilities::AddVector(myLinePoints, {}, {scaledHalfLength * 0.5f, 0.f, 0.f},
                                  aCameraPosition, {1.f, 0.f, 0.f, 1.f}, 1.f);
		

        // y
        myLinePoints.emplace_back(glm::vec3{},glm::vec4{0.f, 1.f, 0.f, 1.f});
        myLinePoints.emplace_back(glm::vec3{0.f, scaledHalfLength, 0.f},glm::vec4{0.f, 1.f, 0.f, 1.f});
        DebugUtilities::AddVector(myLinePoints, {}, {0.f, scaledHalfLength * 0.5f, 0.f},
                                  aCameraPosition, {0.f, 1.f, 0.f, 1.f}, 1.f);

        // z
        myLinePoints.emplace_back(glm::vec3{0.f, 0.f, -scaledHalfLength},glm::vec4{0.f, 0.f, 1.f, 1.f});
        myLinePoints.emplace_back(glm::vec3{0.f, 0.f, scaledHalfLength},glm::vec4{0.f, 0.f, 1.f, 1.f});
        DebugUtilities::AddVector(myLinePoints, {}, {0.f, 0.f, scaledHalfLength * 0.5f},
                                  aCameraPosition, {0.f, 0.f, 1.f, 1.f}, 1.f);

        // �
    }
}

const std::vector<Vulkan::LineVertex>& Wonder::LineGrid::GetLinePoints() const
{
    return myLinePoints;
}
