#pragma once
#include "ECS/Entity.h"
#include "ECS/Reflection/ComponentReflectionUtil.h"
#include <glm/vec4.hpp>

#include "AssetManagement/AssetRegistry.h"

struct ModelComponent
{
    Wonder::ModelId modelId;
    Wonder::MaterialId materialId = Wonder::TEST_MATERIAL_1;
    glm::vec4 customData;
    bool isVisible;
};

template<>
inline void ECS::OnReflectComponent(ModelComponent& aComponent)
{
    Wonder::Reflection::ReflectProperty(aComponent.modelId, "Model");
    Wonder::Reflection::ReflectProperty(aComponent.materialId, "Material");
    Wonder::Reflection::ReflectProperty(aComponent.isVisible, "Is Visible");
    Wonder::Reflection::ReflectProperty(aComponent.customData, "Custom Data");
}