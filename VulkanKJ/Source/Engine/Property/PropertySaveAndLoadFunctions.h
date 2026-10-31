#pragma once
#include <tuple>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "CustomPropertyStructs.h"
#include "Util/StringRegistry.h"

// I don't use static variables to temporary store property names here
// because I might want multithreading on this in the future

#pragma region Save

namespace Goose
{
	class FunctionCurve;
	class Gradient;
}

template<typename T>
concept is_enum = std::is_enum_v<T>;

void SaveProperty(const bool& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const int& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const uint32_t& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const float& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const glm::vec2& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const glm::vec3& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const glm::vec4& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const Wonder::ColorProperty& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const std::string& aValue, nlohmann::json& aJsonData, const char* aValueName);
void SaveProperty(const Tga::StringId& aValue, nlohmann::json& aJsonData, const char* aValueName);

template<typename T>
void SaveProperty(const std::vector<T>& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	nlohmann::json elementsJson;
	std::string elementName;
	for (int i = 0; i < static_cast<int>(aValue.size()); ++i)
	{
		elementName = "element ";
		elementName.append(std::to_string(i));
		SaveProperty(aValue[i], elementsJson, elementName.c_str());
	}
	elementsJson["!size"] = static_cast<int>(aValue.size());
	aJsonData[aValueName] = elementsJson;
}

template<typename T, std::size_t N>
static void SaveProperty(const std::array<T, N>& aValue, nlohmann::json& aJsonData, const char* aValueName);

template<typename T, std::size_t N>
void SaveProperty(const std::array<T, N>& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	nlohmann::json elementsJson;
	std::string elementName;
	for (int i = 0; i < static_cast<int>(aValue.size()); ++i)
	{
		elementName = "element ";
		elementName.append(std::to_string(i));
		SaveProperty(aValue[i], elementsJson, elementName.c_str());
	}
	elementsJson["!size"] = static_cast<int>(aValue.size());
	aJsonData[aValueName] = elementsJson;
}

#define DECLARE_SAVE_WRAPPED_PROPERTY(TYPE,VAR_NAME) inline void SaveProperty(const TYPE& aValue, nlohmann::json& aJsonData, const char* aValueName)\
{\
	aJsonData[aValueName] = aValue.VAR_NAME;\
}

DECLARE_SAVE_WRAPPED_PROPERTY(Wonder::ModelId, id);
DECLARE_SAVE_WRAPPED_PROPERTY(Wonder::MaterialId, id);
DECLARE_SAVE_WRAPPED_PROPERTY(Wonder::TextureId, id);

#pragma endregion

#pragma region Load

#define LOAD_SAFE_GUARD(JsonData,ValueName) if((JsonData).contains(ValueName) == false) return;

void LoadProperty(bool& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(int& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(uint32_t& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(float& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(glm::vec2& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(glm::vec3& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(glm::vec4& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(Wonder::ColorProperty& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(std::string& aValue, const nlohmann::json& aJsonData, const char* aValueName);
void LoadProperty(Tga::StringId& aValue, const nlohmann::json& aJsonData, const char* aValueName);

template<typename T>
static void LoadProperty(std::vector<T>& aValue, const nlohmann::json& aJsonData, const char* aValueName);

template<typename T>
void LoadProperty(std::vector<T>& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	const nlohmann::json& elementsJson = aJsonData[aValueName];
	aValue.resize(elementsJson["!size"]);
	std::string elementName;

	for (int i = 0; i < static_cast<int>(aValue.size()); ++i)
	{
		elementName = "element ";
		elementName.append(std::to_string(i));
		LoadProperty(aValue[i], elementsJson, elementName.c_str());
	}
}

template<typename T, std::size_t N>
static void LoadProperty(std::array<T, N>& aValue, const nlohmann::json& aJsonData, const char* aValueName);

template<typename T, std::size_t N>
void LoadProperty(std::array<T, N>& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	const nlohmann::json& elementsJson = aJsonData[aValueName];
	std::string elementName;

	for (int i = 0; i < static_cast<int>(aValue.size()); ++i)
	{
		elementName = "element ";
		elementName.append(std::to_string(i));
		LoadProperty(aValue[i], elementsJson, elementName.c_str());
	}
}

#define DECLARE_LOAD_WRAPPED_PROPERTY(TYPE,VAR_NAME) inline void LoadProperty(TYPE& aValue, const nlohmann::json& aJsonData, const char* aValueName)\
{\
	LOAD_SAFE_GUARD(aJsonData, aValueName);\
	aValue.VAR_NAME = aJsonData[aValueName];\
}

DECLARE_LOAD_WRAPPED_PROPERTY(Wonder::ModelId, id);
DECLARE_LOAD_WRAPPED_PROPERTY(Wonder::MaterialId, id);
DECLARE_LOAD_WRAPPED_PROPERTY(Wonder::TextureId, id);

#pragma endregion
