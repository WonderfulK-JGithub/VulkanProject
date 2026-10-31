#pragma once

#include <nlohmann/json.hpp>

namespace std::filesystem
{
	class path;
}

namespace FileFunctions
{
	bool CanDeleteFile(const std::filesystem::path& aFilePath);
	bool CanBeRenamed(const std::filesystem::path& aFilePath);

	void SetToAppdataPath(std::filesystem::path& aFilePath);
	std::filesystem::path GetAppdataPath();

	void LoadFileToJson(const std::filesystem::path& aPath, nlohmann::json& aJson);
	void SaveFileFromJson(const std::filesystem::path& aPath, const nlohmann::json& aJson);
	bool CopyToClipboard(const char* text);
	
	long long GetFileTimeStamp(const std::filesystem::path& aFilePath);
}
