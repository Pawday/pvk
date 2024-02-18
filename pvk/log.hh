#pragma once

#include <string>

namespace log {

void error(const std::string &message) noexcept;
void warning(const std::string &message) noexcept;
void info(const std::string &message) noexcept;
void debug(const std::string &message) noexcept;
void trace(const std::string &message, const std::string &source = "") noexcept;

// You need to include std::format to use this macro
#define PVK_TRACE(message) log::trace(message, std::format("{}:{}", __FILE_NAME__, __LINE__))

} // namespace log
