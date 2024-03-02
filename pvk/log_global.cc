#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "log_utils.hh"

namespace pvk {

constexpr std::string ansi_color(uint8_t r, uint8_t g, uint8_t b)
{
    return std::format("\x1B[38;2;{};{};{}m", r, g, b);
};

constexpr std::string ansi_reset()
{
    return "\x1B[0m";
}

void log_lines_to(
    std::ostream &stream,
    const std::string prefix,
    const std::string &message,
    const std::vector<Segment> &segments
)
{
    for (auto segment : segments) {
        std::string_view line(message.data() + segment.start, segment.size);
        stream << std::format("{}{}\n", prefix, line);
    }
}

void error(const std::string &message) noexcept
try {
    std::cerr << ansi_color(0xff, 0, 0);
    auto lines = split_ln(message);
    log_lines_to(std::cerr, "[pvk] [ERROR] ", message, lines);
    std::cerr << ansi_reset();
} catch (...) {
}

void warning(const std::string &message) noexcept
try {
    std::cerr << ansi_color(0xf6, 0xff, 0x00);
    auto lines = split_ln(message);
    log_lines_to(std::cout, "[pvk] [WARN ] ", message, lines);
    std::cerr << ansi_reset();
} catch (...) {
}

void info(const std::string &message) noexcept
try {
    auto lines = split_ln(message);
    log_lines_to(std::cout, "[pvk] [INFO ] ", message, lines);
} catch (...) {
}

void debug(const std::string &message) noexcept
try {
    std::cout << ansi_color(0x2e, 0x86, 0xc9);
    auto lines = split_ln(message);
    log_lines_to(std::cout, "[pvk] [DEBUG] ", message, lines);
    std::cout << ansi_reset();
} catch (...) {
}

void trace(const std::string &message, const std::string &source = "") noexcept
try {
    std::cerr << ansi_color(0x00, 0x55, 0x00);
    auto lines = split_ln(message);
    log_lines_to(
        std::cerr,
        std::format("{}{} ", "[pvk] [TRACE] ", source),
        message,
        lines
    );
    std::cerr << ansi_reset();
} catch (...) {
}

} // namespace log
