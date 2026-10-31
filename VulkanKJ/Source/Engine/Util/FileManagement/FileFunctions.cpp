#include "FileFunctions.h"

#include <cstring>  
#include <filesystem>
#include <fstream>
#include <iostream>
#include <windows.h>  
#include <shlobj.h>

#include "Util/Logging/Logging.h"

bool FileFunctions::CanDeleteFile(const std::filesystem::path& aFilePath)
{
	namespace fs = std::filesystem;

	CanBeRenamed(aFilePath);
	const fs::path parent = aFilePath.parent_path();

	// Try to create and delete a temporary file in the parent directory
	try
	{
		const fs::path dummy = parent / ".__delete_test__";
		std::ofstream test(dummy.string());
		if (!test)
		{
			return false;
		}
		test.close();
		fs::remove(dummy);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool FileFunctions::CanBeRenamed(const std::filesystem::path& aFilePath)
{
	namespace fs = std::filesystem;

	if (!exists(aFilePath))
	{
		LOG_ERROR("Path does not exist: %s", aFilePath.string().c_str());
		return false;
	}

	const fs::path parent = aFilePath.parent_path();
	if (!exists(parent))
	{
		LOG_ERROR("%s","Parent directory does not exist");
		return false;
	}

	// Try to rename and rename back to check permissions
	fs::path temp_path = aFilePath;
	temp_path += ".tmp_check";

	try
	{
		fs::rename(aFilePath, temp_path);
		fs::rename(temp_path, aFilePath);
	}
	catch (const fs::filesystem_error& e)
	{
		LOG_ERROR("Rename failed: %s", e.what());
		return false;
	}

	return true;
}

void FileFunctions::SetToAppdataPath(std::filesystem::path& aFilePath)
{
	wchar_t* localAppDataFolder = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &localAppDataFolder) != S_OK) {
		std::cerr << "problem getting appdata folder\n";
	}

	aFilePath = std::filesystem::path(localAppDataFolder) / "WonderfulEngine";

	if (std::filesystem::exists(aFilePath) == false)
	{
		std::filesystem::create_directory(aFilePath);
	}
}

std::filesystem::path FileFunctions::GetAppdataPath()
{
	std::filesystem::path path;
	SetToAppdataPath(path);
	return path;
}

void FileFunctions::LoadFileToJson(const std::filesystem::path& aPath, nlohmann::json& aJson)
{
	//PROFILE_FUNCTION();

	if (aJson.empty() == false)
	{
		aJson.clear();
	}
	if (exists(aPath) == false)
	{
		LOG_ERROR("File %s does not exist", aPath.string().c_str());
		return;
	}
	
	
	std::ifstream inputStream(aPath);
	try
	{
		//PROFILE_SCOPE("Load file as json");
		inputStream >> aJson;
	}
	catch (const nlohmann::json::parse_error& e)
	{
		LOG_ERROR("File load file %s, JSON parse error: %s",aPath.filename().c_str(), e.what());
	}
}

void FileFunctions::SaveFileFromJson(const std::filesystem::path& aPath, const nlohmann::json& aJson)
{
	//PROFILE_FUNCTION();
	if (std::filesystem::exists(aPath))
	{
		std::filesystem::permissions(aPath, std::filesystem::perms::all, std::filesystem::perm_options::replace);
	}

	std::ofstream outPut(aPath, std::ios::out | std::ios::trunc);
	if (outPut.is_open())
	{
		outPut << aJson.dump(2);
		outPut.close();
	}
	else
	{
		LOG_ERROR("Failed to write to file: %s", aPath.string().c_str());
	}
}

bool FileFunctions::CopyToClipboard(const char* text)
{
	// Open clipboard (NULL = current thread's window)  
	if (!OpenClipboard(nullptr))
	{
		MessageBoxA(nullptr, "Failed to open clipboard", "Error", MB_ICONERROR);
		return false;
	}

	// Clear existing clipboard data  
	if (!EmptyClipboard())
	{
		MessageBoxA(nullptr, "Failed to empty clipboard", "Error", MB_ICONERROR);
		CloseClipboard();
		return false;
	}

	// Allocate global memory for the text (GMEM_MOVEABLE = movable, GMEM_ZEROINIT = zero-initialized)  
	const size_t textLen  = strlen(text) + 1; // +1 for null terminator  
	const HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, textLen);
	if (hGlobal == nullptr)
	{
		MessageBoxA(nullptr, "Failed to allocate memory", "Error", MB_ICONERROR);
		CloseClipboard();
		return false;
	}

	// Lock global memory to get a pointer to the data  
	char* pGlobal = static_cast<char*>(GlobalLock(hGlobal));
	if (pGlobal == nullptr)
	{
		MessageBoxA(nullptr, "Failed to lock memory", "Error", MB_ICONERROR);
		GlobalFree(hGlobal);
		CloseClipboard();
		return false;
	}

	// Copy text to global memory  
	strcpy_s(pGlobal, textLen, text); // Safe string copy  

	// Unlock global memory  
	GlobalUnlock(hGlobal);

	// Set clipboard data (CF_TEXT = plain text format)  
	if (SetClipboardData(CF_TEXT, hGlobal) == nullptr)
	{
		MessageBoxA(nullptr, "Failed to set clipboard data", "Error", MB_ICONERROR);
		GlobalFree(hGlobal);
		CloseClipboard();
		return false;
	}

	// Close clipboard (releases lock)  
	CloseClipboard();
	return true;
}

long long FileFunctions::GetFileTimeStamp(const std::filesystem::path& aFilePath)
{
	if (!std::filesystem::exists(aFilePath))
	{
		LOG_WARNING("No time stamp found for %s, file does not exist", aFilePath.string().c_str());
		return 0;
	}
	return std::filesystem::last_write_time(aFilePath).time_since_epoch().count();
}

