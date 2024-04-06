#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace pvk::utils {

struct StringPack
{
    StringPack(const StringPack &) = delete;
    StringPack &operator=(const StringPack &) = delete;

    StringPack(StringPack &) = default;
    StringPack &operator=(StringPack &&) = default;

    template <size_t EXTEND = std::dynamic_extent>
    static constexpr std::optional<StringPack>
        create(const std::span<std::string_view, EXTEND> &strings) noexcept
    try {
        const size_t total_size = [&strings]() {
            size_t output = 0;
            for (auto &s : strings) {
                output += s.size() + 1;
            }
            return output;
        }();

        std::vector<size_t> offsets;
        offsets.reserve(strings.size());
        std::vector<char> data;
        data.reserve(total_size);

        size_t offset = 0;

        for (auto &s : strings) {
            offsets.emplace_back(offset);
            std::copy(begin(s), end(s), std::back_inserter(data));
            data.emplace_back(0);
            offset += s.size() + 1;
        }

        // Just in case
        data.emplace_back(0);

        StringPack output;
        output.data = std::move(data);
        output.offsets = std::move(offsets);
        return std::make_optional(output);

    } catch (...) {
        return std::nullopt;
    }

    ~StringPack() = default;

    std::vector<const char *> get() &
    {
        std::vector<const char *> output;
        output.reserve(offsets.size());
        auto make_pointer = [this](size_t offset) -> const char * {
            return data.data() + offset;
        };
        auto pointers = offsets | std::views::transform(make_pointer);
        std::ranges::copy(pointers, std::back_inserter(output));
        return output;
    }

  private:
    StringPack() noexcept = default;
    std::vector<char> data;
    std::vector<size_t> offsets;
};

} // namespace pvk::utils
