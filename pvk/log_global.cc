#include <cstdint>
#include <format>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include <pvk/log.hh>

#include "pvk/internal/log_utils.hh"

#if defined(__clang__)
#define CONSTEXPR_IF_CLANG constexpr
#else
#define CONSTEXPR_IF_CLANG
#endif

#define ANSI_COLORS

namespace pvk {

#if defined(ANSI_COLORS)
CONSTEXPR_IF_CLANG std::string ansi_color(uint8_t r, uint8_t g, uint8_t b)
{
    return std::format("\x1B[38;2;{};{};{}m", r, g, b);
};

CONSTEXPR_IF_CLANG std::string ansi_reset()
{
    return "\x1B[0m";
}
#else
CONSTEXPR_IF_CLANG std::string
    ansi_color(uint8_t /*r*/, uint8_t /*g*/, uint8_t /*b*/)
{
    return "";
};

CONSTEXPR_IF_CLANG std::string ansi_reset()
{
    return "";
}
#endif

void log_lines_to(
    std::ostream &stream,
    const std::string_view prefix,
    const std::string_view suffix,
    const std::string_view &message,
    const std::vector<Segment> &segments)
{
    for (auto segment : segments) {
        std::string_view line(message.data() + segment.start, segment.size);
        stream << std::format("{}{}{}\n", prefix, line, suffix);
    }
}

void raw_fatal(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix = std::format("{}[F]", ansi_color(0xee, 0, 0));
    log_lines_to(std::cout, prefix, ansi_reset(), message, lines);
} catch (...) {
}

void raw_error(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix =
        std::format("{}[E]{}", ansi_color(0xff, 0, 0), ansi_reset());
    log_lines_to(std::cout, prefix, message, "", lines);
} catch (...) {
}

void raw_warning(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);

    std::string prefix =
        std::format("{}[W]{}", ansi_color(0xf6, 0xff, 0x00), ansi_reset());
    log_lines_to(std::cout, prefix, "", message, lines);
} catch (...) {
}

void raw_notice(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix =
        std::format("{}[N]{}", ansi_color(0x70, 0xcb, 0xff), ansi_reset());
    log_lines_to(std::cout, prefix, "", message, lines);
} catch (...) {
}

void raw_info(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    log_lines_to(std::cout, "[I]", "", message, lines);
} catch (...) {
}

void raw_debug(const std::string_view &message) noexcept
try {
    std::string prefix = std::format("{}[D]", ansi_color(0x2e, 0x55, 0xff));
    auto lines = split_ln(message);
    log_lines_to(std::cout, prefix, ansi_reset(), message, lines);
} catch (...) {
}

void raw_trace(const std::string_view &message) noexcept
try {
    auto lines = split_ln(message);
    std::string prefix_mangled =
        std::format("{}[T]", ansi_color(0x00, 0xff, 0x00));
    log_lines_to(std::cout, prefix_mangled, ansi_reset(), message, lines);
} catch (...) {
}

} // namespace pvk
