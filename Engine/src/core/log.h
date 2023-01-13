#pragma once

#include "core/time.h"
#include "core/types.h"
#include "core/assert.h"

#include <fmt/core.h>

#include <string_view>
#include <filesystem>
#include <cassert>
#include <sstream>
#include <mutex>
#include <iostream>
#include <fstream>

namespace ddls {
/**
 * @brief A logger that can write to std out and a file
 * 
 */
class DDLS_API Log
{
public:
    class Styles {
    public:
        static constexpr std::string_view Default = "\033[0m";
        static constexpr std::string_view Bold = "\033[1m";
        static constexpr std::string_view Dim = "\033[2m";
        static constexpr std::string_view Underlined = "\033[4m";
        static constexpr std::string_view Blink = "\033[5m";
        static constexpr std::string_view Reverse = "\033[7m";
        static constexpr std::string_view Hidden = "\033[8m";
    };

    class Colours {
    public:
        static constexpr std::string_view Default = "\033[39m";
        static constexpr std::string_view Black = "\033[30m";
        static constexpr std::string_view Red = "\033[31m";
        static constexpr std::string_view Green = "\033[32m";
        static constexpr std::string_view Yellow = "\033[33m";
        static constexpr std::string_view Blue = "\033[34m";
        static constexpr std::string_view Magenta = "\033[35m";
        static constexpr std::string_view Cyan = "\033[36m";
        static constexpr std::string_view LightGrey = "\033[37m";
        static constexpr std::string_view DarkGrey = "\033[90m";
        static constexpr std::string_view LightRed = "\033[91m";
        static constexpr std::string_view LightGreen = "\033[92m";
        static constexpr std::string_view LightYellow = "\033[93m";
        static constexpr std::string_view LightBlue = "\033[94m";
        static constexpr std::string_view LightMagenta = "\033[95m";
        static constexpr std::string_view LightCyan = "\033[96m";
        static constexpr std::string_view White = "\033[97m";
    };

    class Level {
    public:
        static constexpr std::string_view Debug = "DEBUG";
        static constexpr std::string_view Info = "INFO";
        static constexpr std::string_view Warning = "WARNING";
        static constexpr std::string_view Error = "ERROR";
        static constexpr std::string_view Assert = "ASSERT";
    };

    static constexpr auto TimestampFormat = "%H:%M:%S";

    /**
     * Outputs a message into the console.
     * @tparam Args The value types to write.
     * @param style The style to output as.
     * @param colour The colour to output as.
     * @param args The values to write.
     */
    template<typename ... Args>
    static void Out(const std::string_view &style, const std::string_view &colour, const std::string_view &level, Args ... args) {
        Write(style, colour, fmt::format("[{} - {}]: ", Time::GetDateTime(TimestampFormat), level), args..., '\n', Styles::Default);
    }

    /**
     * Outputs a debug message into the console.
     * @tparam Args The value types to write.
     * @param args The values to write.
     */
    template<typename ... Args>
    static void Debug(Args ... args) {
    #ifdef DDLS_DEBUG
        Out(Styles::Default, Colours::LightBlue, Level::Debug, args...);
    #else
        ignore(args);
    #endif
    }

    /**
     * Outputs a info message into the console.
     * @tparam Args The value types to write.
     * @param args The values to write.
     */
    template<typename ... Args>
    static void Info(Args ... args) {
        Out(Styles::Default, Colours::Green, Level::Info, args...);
    }

    /**
     * Outputs a warning message into the console.
     * @tparam Args The value types to write.
     * @param args The values to write.
     */
    template<typename ... Args>
    static void Warning(Args ... args) {
        Out(Styles::Default, Colours::Yellow, Level::Warning, args...);
    }

    /**
     * Outputs a error message into the console.
     * @tparam Args The value types to write.
     * @param args The values to write.
     */
    template<typename ... Args>
    static void Error(Args ... args) {
        Out(Styles::Bold, Colours::Red, Level::Error, args...);
    }

    /**
     * Outputs a assert message into the console.
     * @tparam Args The value types to write.
     * @param expr The expression to assertion check.
     * @param args The values to write.
     */
    template<typename ... Args>
    static inline void _Assert(b8 expr, const char* file, u32 line, Args ... args) {
        if (!expr) {
            Out(Styles::Default, Colours::Magenta, Level::Assert, args...);
            ASSERT_TRACE(expr, file, line);
        }
    }

#define Assert(expr, args) Log::_Assert(expr, __FILE__, __LINE__, args)

    static void OpenLog(const std::filesystem::path &filepath);
    static void CloseLog();

private:
    static std::mutex WriteMutex;
    static std::ofstream FileStream;

    /**
     * A internal method used to write values to the out stream and to a file.
     * @tparam Args The value types to write.
     * @param args The values to write.
     */
    template<typename ... Args>
    static void Write(Args ... args) {
        std::unique_lock<std::mutex> lock(WriteMutex);
        
        ((std::cout << std::forward<Args>(args)), ...);
        if (FileStream.is_open()) {
            ((FileStream << std::forward<Args>(args)), ...);
        }
    }
};
}