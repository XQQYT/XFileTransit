#ifndef DEBUG_OUTPUTER_H
#define DEBUG_OUTPUTER_H

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

// 日志级别
enum class LogLevel
{
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR
};

#ifdef DEBUG_OUTPUT

// 跨平台颜色定义
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
class ConsoleColor
{
private:
    static WORD getDefaultColor()
    {
        static WORD defaultColor = []()
            {
                CONSOLE_SCREEN_BUFFER_INFO info;
                GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
                return info.wAttributes;
            }();
        return defaultColor;
    }

public:
    static void setColor(WORD color)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }

    static void reset()
    {
        setColor(getDefaultColor());
    }

    static constexpr WORD RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
    static constexpr WORD GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    static constexpr WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    static constexpr WORD BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    static constexpr WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    static constexpr WORD MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    static constexpr WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
};
#else
// Linux 颜色定义
class ConsoleColor
{
public:
    static void setColor(const char* colorCode)
    {
        std::cout << colorCode;
    }

    static void reset()
    {
        std::cout << "\033[0m";
    }

    static constexpr const char* RED = "\033[1;31m";
    static constexpr const char* GREEN = "\033[1;32m";
    static constexpr const char* YELLOW = "\033[1;33m";
    static constexpr const char* BLUE = "\033[1;34m";
    static constexpr const char* MAGENTA = "\033[1;35m";
    static constexpr const char* CYAN = "\033[1;36m";
    static constexpr const char* WHITE = "\033[1;37m";
    static constexpr const char* GRAY = "\033[1;90m";
    static constexpr const char* RESET = "\033[0m";
};
#endif

class DebugOutputer
{
private:
    LogLevel min_level_ = LogLevel::LEVEL_DEBUG;
    bool enabled_ = true;
    bool use_color_ = true;

    static DebugOutputer& instance()
    {
        static DebugOutputer instance;
        return instance;
    }

    DebugOutputer() = default;

    void output(LogLevel level, const std::string& message)
    {
        if (!enabled_ || level < min_level_)
            return;

        const char* level_str = "";
        const char* color_code = "";
        const char* reset_code = "";

#ifdef _WIN32
        WORD color = 0;
#else
        const char* color = "";
#endif

        switch (level)
        {
        case LogLevel::LEVEL_DEBUG:
            level_str = "[DEBUG]";
#ifdef _WIN32
            color = ConsoleColor::BLUE;
#else
            color = ConsoleColor::GRAY;
#endif
            break;
        case LogLevel::LEVEL_INFO:
            level_str = "[INFO]";
#ifdef _WIN32
            color = ConsoleColor::GREEN;
#else
            color = ConsoleColor::GREEN;
#endif
            break;
        case LogLevel::LEVEL_WARNING:
            level_str = "[WARN]";
#ifdef _WIN32
            color = ConsoleColor::YELLOW;
#else
            color = ConsoleColor::YELLOW;
#endif
            break;
        case LogLevel::LEVEL_ERROR:
            level_str = "[ERROR]";
#ifdef _WIN32
            color = ConsoleColor::RED;
#else
            color = ConsoleColor::RED;
#endif
            break;
        }

        if (use_color_)
        {
#ifdef _WIN32
            ConsoleColor::setColor(color);
            std::cout << level_str << " " << message << std::endl;
            ConsoleColor::reset();
#else
            std::cout << color << level_str << " " << message << ConsoleColor::RESET << std::endl;
#endif
        }
        else
        {
            std::cout << level_str << " " << message << std::endl;
        }
    }

public:
    DebugOutputer(const DebugOutputer&) = delete;
    DebugOutputer& operator=(const DebugOutputer&) = delete;

    class Stream
    {
    private:
        std::ostringstream buffer_;
        LogLevel level_;
        bool active_ = true;

    public:
        Stream(LogLevel level) : level_(level) {}

        Stream(Stream&& other) noexcept
            : buffer_(std::move(other.buffer_)), level_(other.level_), active_(other.active_)
        {
            other.active_ = false;
        }

        Stream& operator=(Stream&& other) noexcept
        {
            if (this != &other)
            {
                buffer_ = std::move(other.buffer_);
                level_ = other.level_;
                active_ = other.active_;
                other.active_ = false;
            }
            return *this;
        }

        Stream(const Stream&) = delete;
        Stream& operator=(const Stream&) = delete;

        ~Stream()
        {
            if (active_)
            {
                instance().output(level_, buffer_.str());
            }
        }

        template <typename T>
        Stream& operator<<(const T& value)
        {
            if (active_)
            {
                buffer_ << value;
            }
            return *this;
        }

        void disable() { active_ = false; }
    };

    // 设置最小日志级别
    static void set_min_level(LogLevel level)
    {
        instance().min_level_ = level;
    }

    // 启用/禁用所有日志
    static void enable(bool enable)
    {
        instance().enabled_ = enable;
    }

    // 启用/禁用颜色
    static void enable_color(bool enable)
    {
        instance().use_color_ = enable;
    }

    // 创建日志流
    static Stream log(LogLevel level = LogLevel::LEVEL_DEBUG)
    {
        return Stream(level);
    }

    // 带函数名和行号的快速日志
    static Stream log_func(LogLevel level, const char* func, int line)
    {
        Stream stream(level);
        stream << "[" << func << ":" << line << "] ";
        return stream;
    }

    // 彩色输出函数
    static Stream red() { return log(LogLevel::LEVEL_ERROR); }
    static Stream green() { return log(LogLevel::LEVEL_INFO); }
    static Stream yellow() { return log(LogLevel::LEVEL_WARNING); }
    static Stream blue() { return log(LogLevel::LEVEL_DEBUG); }
};

// 方便的宏
#define DEBUG_LOG(level, ...) \
    DebugOutputer::log(level) << __VA_ARGS__

#define DEBUG_LOG_FUNC(level, ...) \
    DebugOutputer::log_func(level, __FUNCTION__, __LINE__) << __VA_ARGS__

#define DEBUG_STREAM(level) \
    DebugOutputer::log(level)

// 快速日志宏
#define LOG_DEBUG(...) DEBUG_LOG(LogLevel::LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) DEBUG_LOG(LogLevel::LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(...) DEBUG_LOG(LogLevel::LEVEL_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) DEBUG_LOG(LogLevel::LEVEL_ERROR, __VA_ARGS__)

// 带函数信息的宏
#define LOG_DEBUG_FUNC(...) DEBUG_LOG_FUNC(LogLevel::LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO_FUNC(...) DEBUG_LOG_FUNC(LogLevel::LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN_FUNC(...) DEBUG_LOG_FUNC(LogLevel::LEVEL_WARNING, __VA_ARGS__)
#define LOG_ERROR_FUNC(...) DEBUG_LOG_FUNC(LogLevel::LEVEL_ERROR, __VA_ARGS__)

// 彩色输出宏（简写）
#define LOG_RED(...) DebugOutputer::red() << __VA_ARGS__
#define LOG_GREEN(...) DebugOutputer::green() << __VA_ARGS__
#define LOG_YELLOW(...) DebugOutputer::yellow() << __VA_ARGS__
#define LOG_BLUE(...) DebugOutputer::blue() << __VA_ARGS__

#else
// Release版本
class DebugOutputer
{
public:
    class Stream
    {
    public:
        template <typename T>
        Stream& operator<<(const T&) { return *this; }
    };

    static void set_min_level(LogLevel) {}
    static void enable(bool) {}
    static void enable_color(bool) {}
    static Stream log(LogLevel = LogLevel::LEVEL_DEBUG) { return Stream(); }
    static Stream log_func(LogLevel, const char*, int) { return Stream(); }
    static Stream red() { return Stream(); }
    static Stream green() { return Stream(); }
    static Stream yellow() { return Stream(); }
    static Stream blue() { return Stream(); }
};

// 空宏
#define DEBUG_LOG(level, ...)
#define DEBUG_LOG_FUNC(level, ...)
#define DEBUG_STREAM(level) DebugOutputer::log(level)

#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_DEBUG_FUNC(...)
#define LOG_INFO_FUNC(...)
#define LOG_WARN_FUNC(...)
#define LOG_ERROR_FUNC(...)

#define LOG_RED(...)
#define LOG_GREEN(...)
#define LOG_YELLOW(...)
#define LOG_BLUE(...)

#endif // DEBUG_OUTPUT

#endif // DEBUG_OUTPUTER_H