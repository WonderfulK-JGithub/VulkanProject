#include "Logging.h"

#ifndef _RETAIL

#define FMT_UNICODE 0
// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wnan-infinity-disabled"
#include <spdlog/sinks/stdout_color_sinks-inl.h>
// #pragma clang diagnostic pop

#pragma comment(lib, "User32.lib")
#include <chrono>

#include <iostream>
#include <winuser.h>
#include <comdef.h>

#include "Engine.h"
#include "magic_enum/magic_enum.hpp"
#include "spdlog/sinks/callback_sink.h"

#include "imgui.h"
#include "Util/DebugUtilities.h"

namespace Goose::Logging
{
    namespace Internal
    {
        struct ConsoleLog
        {
            Severity severity;
            std::string message;
        };
        
        constexpr const char* RED       = "\033[38;5;196m";
        constexpr const char* YELLOW    = "\033[38;5;214m";
        constexpr const char* GREEN     = "\033[38;5;40m";
        constexpr const char* BLUE      = "\033[38;5;33m";
        constexpr const char* GRAY      = "\033[38;5;250m";
        constexpr const char* RESET     = "\033[38;5;255m";
        
        static std::shared_ptr<spdlog::logger> callbackLogger;
        static std::shared_ptr<spdlog::logger> immediateLogger;
        
        static std::vector<ConsoleLog> consoleLogBuffer;
        static Flags<Severity> logFilter = Severity::Info | Severity::Warning | Severity::Error | Severity::Assertion;
        
        static Severity LogLevelToSeverity(const spdlog::level::level_enum aLevel)
        {
            switch (aLevel)
            {
                case spdlog::level::info:       return Severity::Info;
                case spdlog::level::warn:       return Severity::Warning;
                case spdlog::level::err:        return Severity::Error;
                case spdlog::level::critical:   return Severity::Assertion;
                case spdlog::level::debug:  
                case spdlog::level::trace:
                case spdlog::level::off:
                case spdlog::level::n_levels:
                {
                    GOOSE_ASSERT(false, "Invalid severity");
                    break;
                }
            }
            
            return Severity::Assertion;
        }
        
        static std::string GetSeverityString(const Severity aSeverity, const bool aColor = true)
        {
            switch (aSeverity)
            {
                case Severity::Info:
                {
                    return aColor ? std::format("[{}Info{}]", GREEN, RESET) : "[Info]";
                }
                case Severity::Warning:
                {
                    return aColor ? std::format("[{}Warn{}]", YELLOW, RESET) : "[Warn]";
                }
                case Severity::Error:
                {
                    return aColor ? std::format("[{}Error{}]", RED, RESET) : "[Error]";
                }
                case Severity::Assertion:
                {
                    return aColor ? std::format("[{}Assert{}]", RED, RESET) : "[Assert]";
                }
            }
            
            return "";
        }

        static void LogCallback(const spdlog::details::log_msg& aMsg)
        {
            auto time = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
            
            Severity s = LogLevelToSeverity(aMsg.level);
            
            const std::string timeStr = std::format("[{}{:%OH:%OM:%OS}{}]", BLUE, time, RESET);
            const std::string timeStrNoColor = std::format("[{:%OH:%OM:%OS}]", time);
            const std::string severity = GetSeverityString(s);
            const std::string log = StringUtils::Format("%s%s: %s", timeStr.c_str(), severity.c_str(), aMsg.payload.data());
            
            const std::string timeSeverity = StringUtils::Format("%s%s: ", timeStrNoColor.c_str(), GetSeverityString(s, false).c_str());
            
            consoleLogBuffer.emplace_back(LogLevelToSeverity(aMsg.level), 
                StringUtils::Format("%s %s", timeSeverity.c_str(), aMsg.payload.data()));
            
            immediateLogger->info(log);
        }

        static bool DisplayMessageBoxHr(const long aHr, const char* aFile, int32_t aLine, const std::string& aMessage)
        {
            const std::string message = StringUtils::Format("HRESULT: 0x%08x\n\nLine: %d\nFile: %s\n\n%s", aHr, aLine, aFile, aMessage.c_str());
            return MessageBoxA(nullptr, message.c_str(), "Assertion Failure!", MB_ICONERROR | MB_RETRYCANCEL) == IDRETRY;
        }
        
        void Init()
        {
            std::cout << RESET;
            
            callbackLogger = spdlog::callback_logger_mt("Goose", LogCallback);
            immediateLogger = spdlog::stdout_color_mt("Logger");
            immediateLogger->set_pattern("%v");
        }

        void Log(const Severity aSeverity, [[maybe_unused]] const char* aFile, [[maybe_unused]] int32_t aLine, const std::string& aMessage)
        {
            switch (aSeverity)
            {
                case Severity::Info:
                {
                    callbackLogger->info(aMessage);
                    break;
                }
                case Severity::Warning:
                {
                    callbackLogger->warn(aMessage);
                    break;
                }
                case Severity::Error:
                {
                    callbackLogger->error(aMessage);
                    break;
                }
                case Severity::Assertion:
                {
                    callbackLogger->critical(aMessage);
                    break;
                }
            }
        }

        void LogDebug(const char*, int32_t, const std::string& aMessage)
        {
            auto time = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
            
            const std::string timeStr = std::format("[{}{:%OH:%OM:%OS}{}]", BLUE, time, RESET);
            const std::string timeStrNoColor = std::format("[{:%OH:%OM:%OS}]", time);
            const std::string severity = std::format("[{}Debug{}]", GRAY, RESET);
            const std::string log = StringUtils::Format("%s%s: %s", timeStr.c_str(), severity.c_str(), aMessage.c_str());
            
            immediateLogger->info(log);
        }

        bool DisplayAssertMessageBox(const char* aFile, int32_t aLine, const std::string& aMessage)
        {
            const std::string message = StringUtils::Format("Line: %d\nFile: %s\n\n%s", aLine, aFile, aMessage.c_str());
            return MessageBoxA(nullptr, message.c_str(), "Assertion Failure!", MB_ICONERROR | MB_RETRYCANCEL) == IDRETRY;
        }

        void DisplayConsole()
        {
            static bool testPrint = false;

            if (ImGui::Begin("Console"))
            {
                if (ImGui::Button("Clear"))
                {
                    consoleLogBuffer.clear();
                }

                ImGui::SameLine();

                auto names = magic_enum::enum_names<Severity>();
                if (ImGui::BeginCombo("##Filter", "Filter"))
                {
                    for (auto& name : names)
                    {
                        const Severity severity = magic_enum::enum_cast<Severity>(name).value();
                        bool isSelected = logFilter & severity;
                        if (ImGui::Checkbox(name.data(), &isSelected))
                        {
                            logFilter = isSelected ? logFilter | severity : logFilter & ~severity;
                        }

                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::SameLine();

                if (ImGui::Button("Test Print"))
                {
                    testPrint = !testPrint;
                }

                ImGui::Separator();

                auto get_color = [](Severity severity) -> ImVec4
                {
                    switch (severity)
                    {
                    case Severity::Info:
                        return { 0.0f, 1.0f, 0.0f, 1.0f };
                    case Severity::Warning:
                        return { 1.0f, 1.0f, 0.0f, 1.0f };
                    case Severity::Error:
                        return { 1.0f, 0.0f, 0.0f, 1.0f };
                    case Severity::Assertion:
                        return { 1.0f, 0.0f, 0.0f, 1.0f };
                    }

                    return { 1.0f, 0.0f, 0.0f, 1.0f };
                };

                for (auto& [severity, message] : consoleLogBuffer)
                {
                    if (logFilter & severity)
                    {
                        ImGui::TextColored(get_color(severity), "%s", message.c_str());
                    }
                }

                if (testPrint)
                {
                    LOG_ERROR("Test Print");
                    LOG_WARNING("Test Print");
                    LOG_INFO("Test Print");
                    LOG_DEBUG("Test Print");
                }
            }
            
            

            ImGui::End();
        }

        void DisplayStats()
        {
            if (DebugUtilities::BeginDebugTabItem("Stats"))
            {
                ImGui::Text("Delta time: %fms", Engine::GetInstance().GetDeltaTime() * 1000.f);
                ImGui::Text("FPS: %d", static_cast<int>(1.f / Engine::GetInstance().GetDeltaTime()));
            }
            DebugUtilities::EndDebugTabItem();
        }

        void Terminate()
        {
            std::exit(EXIT_FAILURE);
            //PostQuitMessage(0);
        }

        bool CheckHresult(const long aHr)
        {
	        return static_cast<long>(aHr) >= 0;
        }
    }

    std::vector<EditorWarning> localWarnings;
    void AddWarning(const EditorWarning& aWarning)
    {
        localWarnings.emplace_back(aWarning);
    }

    void AddWarning(const std::string& aWarningMessage)
    {
        AddWarning({aWarningMessage,{},nullptr});
    }

    std::vector<EditorWarning>& AccessWarnings()
    {
        return localWarnings;
    }

    bool LogHresultAssertion(const long aHr, const char* aFile, int32_t aLine)
    {
        const _com_error err(aHr);
        const std::wstring str  = err.ErrorMessage();

        const std::string message(str.begin(), str.end());

	    return Internal::DisplayMessageBoxHr(aHr, aFile, aLine, message);
    }

    void AppendHresult(const long aHresult)
    {
        const _com_error err(aHresult);
        const std::wstring str = err.ErrorMessage();
        const std::string message(str.begin(), str.end());
        
        Internal::immediateLogger->info(StringUtils::Format("\t%s", message.c_str()));
    }
}

#endif
