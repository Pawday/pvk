#pragma once

#include <format>
#include <string>
#include <string_view>

namespace pvk {

struct Logger
{
    enum class Level
    {
        FATAL,
        ERROR,
        WARNING,
        NOTICE,
        INFO,
        DEBUG,
        TRACE
    };

    Logger() = default;
    Logger(Logger &&) = default;
    ~Logger() = default;

    Logger(const Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;

    using Callback = void (*)(void *, Level, const std::string_view &) noexcept;

    void raw_fatal(const std::string_view &message) const noexcept;
    void raw_error(const std::string_view &message) const noexcept;
    void raw_warning(const std::string_view &message) const noexcept;
    void raw_notice(const std::string_view &message) const noexcept;
    void raw_info(const std::string_view &message) const noexcept;
    void raw_debug(const std::string_view &message) const noexcept;
    void raw_trace(const std::string_view &message) const noexcept;

    template <typename... Args>
    void fatal(std::format_string<Args...> fmt, Args &&...args) noexcept
    {
        raw_fatal(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args &&...args) noexcept
    {
        raw_error(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void warning(std::format_string<Args...> fmt, Args &&...args) noexcept
    {
        raw_warning(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void notice(std::format_string<Args...> fmt, Args &&...args) noexcept
    {
        raw_notice(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args &&...args) noexcept
    {
        raw_info(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args &&...args) noexcept
    {
        raw_debug(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void trace(std::format_string<Args...> fmt, Args &&...args) noexcept
    {
        raw_trace(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    void set_name(const std::string_view &source_name) noexcept;
    void set_callback(Callback new_callback) noexcept;
    void set_userdata(void *user_data) noexcept
    {
        m_user_data = user_data;
    }

  private:
    struct Detail;
    Callback m_callback = nullptr;
    void *m_user_data = nullptr;
    std::string m_source_name;
};

} // namespace pvk
