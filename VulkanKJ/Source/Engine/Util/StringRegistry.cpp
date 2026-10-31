
#include "StringRegistry.h"

#include <array>
#include <cassert>
#include <mutex>
#include <unordered_set>

using namespace Tga;

constexpr int STRING_REGISTRY_SIZE_LIMIT = 256;
constexpr int STRING_REGISTRY_BUFFER_SIZE = 1024 * 1024;

static std::vector<std::unique_ptr<std::array<char, STRING_REGISTRY_BUFFER_SIZE>>> locDeduplicatedStringStorage;
static size_t locRemainingBufferSpace;

static std::unordered_set<std::string_view> localDuplicatedStrings;

std::mutex localMutex;

StringId Tga::operator""_tgaid(const char* aString, std::size_t aSize)
{
	if (aSize == 0)
		return {};

	auto it = localDuplicatedStrings.find(aString);
	if (it != localDuplicatedStrings.end())
		return StringId(it->data());

	localDuplicatedStrings.insert(aString);
	return StringId(aString);
}

StringId Tga::StringRegistry::RegisterOrGetString(std::string_view aString)
{
	std::lock_guard safeLock(localMutex);
	// String registry is meant for smaller strings
	if (aString.size() + 1 > STRING_REGISTRY_SIZE_LIMIT)
	{
		return {};
	}

	if (aString.empty())
	{
		return {};
	}

	auto it = localDuplicatedStrings.find(aString);
	if (it != localDuplicatedStrings.end())
		return StringId(it->data());

	if (aString.size() + 1 > locRemainingBufferSpace)
	{
		locRemainingBufferSpace = STRING_REGISTRY_BUFFER_SIZE;
		locDeduplicatedStringStorage.push_back(std::make_unique<std::array<char, STRING_REGISTRY_BUFFER_SIZE>>());
	}

	char* stringLocation = locDeduplicatedStringStorage.back()->data() + (STRING_REGISTRY_BUFFER_SIZE - locRemainingBufferSpace);
	locRemainingBufferSpace -= static_cast<int>(aString.size()) + 1;

	strncpy_s(stringLocation,aString.size() + 1, aString.data(), locRemainingBufferSpace);

	StringId id = { stringLocation };
	localDuplicatedStrings.insert(stringLocation);
	
	return id;
}
