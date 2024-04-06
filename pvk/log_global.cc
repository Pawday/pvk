#include <cstdint>
#include <format>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "pvk/log.hh"
#include "pvk/log_utils.hh"

#if defined(__clang__)
#define CONSTEXPR_IF_CLANG constexpr
#else
#define CONSTEXPR_IF_CLANG
#endif

namespace pvk {

CONSTEXPR_IF_CLANG std::string ansi_color(uint8_t r, uint8_t g, uint8_t b)
{
    return std::format("\x1B[38;2;{};{};{}m", r, g, b);
};

CONSTEXPR_IF_CLANG std::string ansi_reset()
{
    return "\x1B[0m";
}

void log_lines_to(
    std::ostream &stream,
    const std::string_view prefix,
    const std::string_view &message,
    const std::vector<Segment> &segments)
{
    for (auto segment : segments) {
        std::string_view line(message.data() + segment.start, segment.size);
        stream << std::format("{}{}\n", prefix, line);
    }
}

void fatal(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix = std::format("{}[FATAL] ", ansi_color(0xee, 0, 0));
    log_lines_to(std::cerr, prefix, message, lines);
    ansi_reset();
} catch (...) {
}

void error(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix =
        std::format("{}[ERROR]{} ", ansi_color(0xff, 0, 0), ansi_reset());
    log_lines_to(std::cerr, prefix, message, lines);
} catch (...) {
}

void warning(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);

    std::string prefix =
        std::format("{}[WARN ]{} ", ansi_color(0xf6, 0xff, 0x00), ansi_reset());
    log_lines_to(std::cout, prefix, message, lines);
} catch (...) {
}

void notice(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix =
        std::format("{}[NOTE ]{} ", ansi_color(0xca, 0xfa, 0xee), ansi_reset());
    log_lines_to(std::cout, prefix, message, lines);
} catch (...) {
}

void info(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    log_lines_to(std::cout, "[INFO ] ", message, lines);
} catch (...) {
}

void debug(const std::string_view &message) noexcept
try {
    std::string prefix =
        std::format("{}[DEBUG]{} ", ansi_color(0x2e, 0x55, 0xff), ansi_reset());
    auto lines = split_ln(message);
    log_lines_to(std::cout, prefix, message, lines);
} catch (...) {
}

void trace(
    const std::string_view &message, const std::string_view &source) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix =
        std::format("{}[TRACE]{} ", ansi_color(0x00, 0x55, 0x00), ansi_reset());
    log_lines_to(
        std::cout, std::format("{}{}", prefix, source), message, lines);
} catch (...) {
}

} // namespace pvk
