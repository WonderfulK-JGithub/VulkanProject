#include "PropertySaveAndLoadFunctions.h"

void LoadProperty(bool& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	aValue = aJsonData[aValueName];
}

void LoadProperty(int& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	aValue = aJsonData[aValueName];
}

void LoadProperty(uint32_t& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	aValue = aJsonData[aValueName];
}

void LoadProperty(float& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	aValue = aJsonData[aValueName];
}

void LoadProperty(glm::vec2& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	std::array<float, 2> vectorData = aJsonData[aValueName].get<std::array<float, 2>>();
	aValue                          = {vectorData[0], vectorData[1]};
}

void LoadProperty(glm::vec3& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	std::array<float, 3> vectorData = aJsonData[aValueName].get<std::array<float, 3>>();
	aValue                          = {vectorData[0], vectorData[1], vectorData[2]};
}

void LoadProperty(glm::vec4& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	std::array<float, 4> vectorData = aJsonData[aValueName].get<std::array<float, 4>>();
	aValue                          = { vectorData[0], vectorData[1], vectorData[2], vectorData[3] };
}

void LoadProperty(Wonder::ColorProperty& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	std::array<float, 4> colorData = aJsonData[aValueName].get<std::array<float, 4>>();
	aValue.color                         = {colorData[0], colorData[1], colorData[2], colorData[3]};
}

void LoadProperty(std::string& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	aValue = aJsonData[aValueName];
}

void LoadProperty(Tga::StringId& aValue, const nlohmann::json& aJsonData, const char* aValueName)
{
	LOAD_SAFE_GUARD(aJsonData, aValueName);
	aValue = Tga::StringRegistry::RegisterOrGetString(aJsonData[aValueName]);
}

void SaveProperty(const bool& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = aValue;
}
void SaveProperty(const int& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = aValue;
}

void SaveProperty(const uint32_t& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = aValue;
}

void SaveProperty(const float& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = aValue;
}

void SaveProperty(const glm::vec2& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = {aValue.x, aValue.y};
}

void SaveProperty(const glm::vec3& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = {aValue.x, aValue.y, aValue.z};
}

void SaveProperty(const glm::vec4& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = { aValue.x, aValue.y, aValue.z, aValue.w };
}

void SaveProperty(const Wonder::ColorProperty& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = {aValue.color.r, aValue.color.g, aValue.color.b, aValue.color.a};
}

void SaveProperty(const std::string& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = aValue;
}

void SaveProperty(const Tga::StringId& aValue, nlohmann::json& aJsonData, const char* aValueName)
{
	aJsonData[aValueName] = aValue.GetString();
}

