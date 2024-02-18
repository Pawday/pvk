#pragma once

#include <string>

namespace log {

void error(const std::string &message) noexcept;
void warning(const std::string &message) noexcept;
void info(const std::string &message) noexcept;
void trace(const std::string &message, const std::string &source = "") noexcept;

} // namespace log
