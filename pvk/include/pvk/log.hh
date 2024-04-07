#pragma once

#include <string_view>

namespace pvk {

void fatal(const std::string_view &message) noexcept;
void error(const std::string_view &message) noexcept;
void warning(const std::string_view &message) noexcept;
void notice(const std::string_view &message) noexcept;
void info(const std::string_view &message) noexcept;
void debug(const std::string_view &message) noexcept;
void trace(const std::string_view &message, const std::string_view &prefix = "") noexcept;

// You need to include std::format to use this macro
#define PVK_TRACE(message)                                                     \
    pvk::trace(message, std::format(" {}:{} ", __FILE_NAME__, __LINE__))

} // namespace pvk
