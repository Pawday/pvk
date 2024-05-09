#include <algorithm>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>

#include "pvk/internal/log_utils.hh"

#define ANSI_BOX

namespace {

#if defined(ANSI_BOX)
constexpr char lt[] = "┌";
constexpr char rt[] = "┐";
constexpr char lb[] = "└";
constexpr char rb[] = "┘";
constexpr char vl[] = "│";
constexpr char hl[] = "─";
constexpr char brl[] = "├";
constexpr char brr[] = "┤";
#else
constexpr char lt[] = "+";
constexpr char rt[] = "+";
constexpr char lb[] = "+";
constexpr char rb[] = "+";
constexpr char vl[] = "|";
constexpr char hl[] = "-";
constexpr char brl[] = "+";
constexpr char brr[] = "+";
#endif

std::string box_line(size_t box_width)
{
    std::string stub;
    stub.resize(box_width * (sizeof(hl) - 1));
    for (size_t s = 0; s < box_width; s++) {
        std::copy(
            hl, hl + (sizeof(hl) - 1), stub.begin() + s * (sizeof(hl) - 1));
    }
    return stub;
}
} // namespace

std::string box_title(std::string_view title, size_t box_width)
{
    std::string stub = box_line(box_width);
    std::string head = std::format("{}{}{}{}{}", lt, hl, stub, hl, rt);
    std::string sep = std::format("{}{}{}{}{}", brl, hl, stub, hl, brr);

    std::string title_line =
        std::format("{} {:^{}} {}", vl, title, box_width, vl);

    std::string output;

    return std::format("{}\n{}\n{}", head, title_line, sep);
}

std::string box_entry(std::string_view entry, size_t box_width)
{
    return std::format("{} {:<{}} {}", vl, entry, box_width, vl);
}

std::string box_foot(size_t box_width)
{
    return std::format("{}{}{}{}{}", lb, hl, box_line(box_width), hl, rb);
}
