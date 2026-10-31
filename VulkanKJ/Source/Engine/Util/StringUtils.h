#pragma once
#include <memory>
#include <string>
#include <vector>


// #include <stringapiset.h>
namespace Goose::StringUtils
{
    template<typename... Args>
    static std::string Format(const char* aFormat, Args&&... aArgs)
    {
// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wformat-security"
        
        const int size_s = std::snprintf(nullptr, 0, aFormat, std::forward<Args>(aArgs)...) + 1;

        const auto size = static_cast<size_t>(size_s);

        const std::unique_ptr<char[]> buf(new char[size]);

        std::snprintf(buf.get(), size, aFormat, std::forward<Args>(aArgs)...);
        
        return std::string(buf.get(), buf.get() + size - 1);
        
// #pragma clang diagnostic pop
    }
    
    std::vector<std::string> SplitString(std::string aLine, const std::string& aDelimiter);
    std::vector<std::string> SplitString(const std::vector<std::string>& aTexts, const std::string& aDelimiter);
    std::string GetAfter(std::string aLine, const std::string& aDelimiter);
    std::string GetBefore(std::string aLine, const std::string& aDelimiter);
    
    void RemoveAll(std::string& aText,char aCharacter);
    void RemoveAll(std::string& aText,const char* aCharacters);
}

