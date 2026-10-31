#pragma once
#pragma warning(disable:4722) 
#pragma warning(disable:4244) 
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "Util/Flags.h"
#include "Util/StringUtils.h"

#ifndef _RETAIL
namespace Goose::Logging
{
    struct EditorWarning
    {
        std::string warningMessage;
        std::vector<std::string> choices;
        std::function<void(uint32_t)> callback;
    };
    
    void AddWarning(const EditorWarning& aWarning);
    void AddWarning(const std::string& aWarningMessage);
    std::vector<EditorWarning>& AccessWarnings();
    
    namespace Internal
    {
        enum class Severity : uint8_t
        {
            Info        = BIT(0),
            Warning     = BIT(1),
            Error       = BIT(2),
            Assertion   = BIT(4)
        };
        
        void Init();
        
        void Log(Severity aSeverity, const char* aFile, int32_t aLine, const std::string& aMessage);
        
        void LogDebug(const char* aFile, int32_t aLine, const std::string& aMessage);

        bool DisplayAssertMessageBox(const char* aFile, int32_t aLine, const std::string& aMessage);
        
        void DisplayConsole();

        void DisplayStats();
        
        void Terminate();

        bool CheckHresult(long aHr);
    }
    
    template<typename... Args> 
    void LogDebug(const char* aFile, const int32_t aLine, const char* aFmt, Args&&... aArgs)
    {
        Internal::LogDebug(aFile, aLine, StringUtils::Format(aFmt, std::forward<Args>(aArgs)...));
    }
    
    template<typename... Args> 
    void LogInfo(const char* aFile, const int32_t aLine, const char* aFmt, Args&&... aArgs)
    {
        Internal::Log(Internal::Severity::Info, aFile, aLine, StringUtils::Format(aFmt, std::forward<Args>(aArgs)...));
    }
    
    template<typename... Args>
    void LogWarning(const char* aFile, const int32_t aLine, const char* aFmt, Args&&... aArgs)
    {
        Internal::Log(Internal::Severity::Warning, aFile, aLine, StringUtils::Format(aFmt, std::forward<Args>(aArgs)...));
    }
    
    template<typename... Args>
    void LogError(const char* aFile, const int32_t aLine, const char* aFmt, Args&&... aArgs)
    {
        Internal::Log(Internal::Severity::Error, aFile, aLine, StringUtils::Format(aFmt, std::forward<Args>(aArgs)...));
    }
    
    template<typename... Args>
    int LogAssertion(const char* aFile, const int32_t aLine, const char* aFmt, Args&&... aArgs)
    {
        const std::string message = StringUtils::Format(aFmt, std::forward<Args>(aArgs)...);
        Internal::Log(Internal::Severity::Assertion, aFile, aLine, message);
        return Internal::DisplayAssertMessageBox(aFile, aLine, message);
    }

    bool LogHresultAssertion(long aHr, const char* aFile, int32_t aLine);
    
    void AppendHresult(long aHresult);
}

DEFINE_FLAG_TRAITS(Goose::Logging::Internal::Severity);

#endif

#ifdef _DEBUG

#define GOOSE_ASSERT(expr, fmt, ...) ((void)( (!!(expr)) || ((Goose::Logging::LogAssertion(__FILE__, __LINE__, fmt, ##__VA_ARGS__) ? __debugbreak() : Goose::Logging::Internal::Terminate()), 0)))

#define GOOSE_ASSERT_HRESULT(hr) ((void)( (!!(((long)(hr)) >= 0)) || ((Goose::Logging::LogHresultAssertion(hr, __FILE__, __LINE__) ? __debugbreak() : Goose::Logging::Internal::Terminate()), 0)))

#else

#define GOOSE_ASSERT(expr, fmt, ...)

#define GOOSE_ASSERT_HRESULT(hr)

#endif

#ifndef _RETAIL 

#define INIT_LOGGING() Goose::Logging::Internal::Init()

#define SHOW_CONSOLE_LOGGING() Goose::Logging::Internal::DisplayConsole()

#define LOG_DEBUG(fmt, ...)     Goose::Logging::LogDebug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)      Goose::Logging::LogInfo(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)   Goose::Logging::LogWarning(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)     Goose::Logging::LogError(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#else

#define INIT_LOGGING()
#define SHOW_CONSOLE_LOGGING()
#define LOG_TRACE(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARNING(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#endif

