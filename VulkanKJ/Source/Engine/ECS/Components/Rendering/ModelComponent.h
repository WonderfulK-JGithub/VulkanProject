#pragma once
#include "ECS/Entity.h"
#include "ECS/Reflection/ComponentReflectionUtil.h"
#include <glm/vec4.hpp>

struct ModelComponent
{
    Wonder::ModelId modelId;
    bool isVisible;
    glm::vec4 customData;
};

template<>
inline void ECS::OnReflectComponent(ModelComponent& aComponent)
{
    Wonder::Reflection::ReflectProperty(aComponent.modelId, "Model");
    Wonder::Reflection::ReflectProperty(aComponent.isVisible, "Is Visible");
    Wonder::Reflection::ReflectProperty(aComponent.customData, "Custom Data");
}