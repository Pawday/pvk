#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

struct Segment
{
    size_t start;
    size_t size;
};

inline std::vector<Segment> split_ln(const std::string_view &data)
{
    size_t linebreaks = 0;

    std::ranges::for_each(data, [&linebreaks](char c) {
        if (c == '\n') {
            linebreaks++;
        }
    });

    if (linebreaks == 0) {
        return {{0, data.size()}};
    }

    static std::vector<Segment> output;
    output.clear();
    if (output.capacity() < linebreaks) {
        output.reserve(linebreaks);
    }

    size_t start = 0;
    size_t end = data.find_first_of('\n', start);

    while (end != std::string::npos) {
        if (start > data.size()) {
            break;
        }
        size_t size = end - start;
        output.push_back({start, size});

        start = end + 1;
        end = data.find_first_of('\n', start);
    }

    output.push_back({start, data.size() - start});

    return output;
}
