#include "StringUtils.h"

namespace Goose::StringUtils
{
    std::vector<std::string> SplitString(std::string aLine, const std::string& aDelimiter)
    {
        std::vector<std::string> result;

        size_t pos;
        while ((pos = aLine.find(aDelimiter)) != std::string::npos)
        {
            std::string token = aLine.substr(0, pos);
            result.push_back(token);
            aLine.erase(0, pos + aDelimiter.length());
        }

        result.push_back(aLine);

        return result;
    }

    std::vector<std::string> SplitString(const std::vector<std::string>& aTexts, const std::string& aDelimiter)
    {
        std::vector<std::string> result;
        for (auto& text : aTexts)
        {
            const std::vector<std::string> subResult = SplitString(text, aDelimiter);
            result.insert(result.end(), subResult.begin(), subResult.end());
        }
        return result;
    }

    std::string GetAfter(std::string aLine, const std::string& aDelimiter)
    {
        size_t pos;
        if ((pos = aLine.find(aDelimiter)) != std::string::npos)
        {
            aLine.erase(0, pos + aDelimiter.length());
        }
        
        return aLine;
    }

    std::string GetBefore(std::string aLine, const std::string& aDelimiter)
    {
        size_t pos;
        if ((pos = aLine.find(aDelimiter)) != std::string::npos)
        {
            return aLine.substr(0, pos);
        }
        
        return aLine;
    }

    void RemoveAll(std::string& aText, char aCharacter)
    {
        size_t pos{};
        while (pos < aText.length())
        {
            if (aText[pos] == aCharacter)
            {
                aText.erase(pos,1);
            }
            else
            {
                pos++;
            }
        }
    }

    void RemoveAll(std::string& aText, const char* aCharacters)
    {
        size_t pos{};
        while (pos < aText.length())
        {
            for (uint32_t i = 0; aCharacters[i] != '\0'; ++i)
            {
                if (aText[pos] == aCharacters[i])
                {
                    aText.erase(pos);
                    pos--;
                    break;
                }
            }
            pos++;
        }
    }
}

