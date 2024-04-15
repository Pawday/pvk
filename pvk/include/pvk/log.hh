#pragma once

#include <format>
#include <string_view>

namespace pvk {

void raw_fatal(const std::string_view &message) noexcept;
void raw_error(const std::string_view &message) noexcept;
void raw_warning(const std::string_view &message) noexcept;
void raw_notice(const std::string_view &message) noexcept;
void raw_info(const std::string_view &message) noexcept;
void raw_debug(const std::string_view &message) noexcept;
void raw_trace(const std::string_view &message) noexcept;

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

#define PVK_TRACE(message)                                                     \
    pvk::trace(std::format(" {}:{} {}", __FILE_NAME__, __LINE__, message))

} // namespace pvk
