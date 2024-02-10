#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "dso_loader.hh"

struct VKLoader
{
    struct VKVersion
    {
        using Major_t = uint8_t;
        using Minor_t = uint16_t;
        Major_t major;
        Minor_t minor;
    };

    using Optional = std::optional<VKLoader>;
    static Optional load(const std::string &library) noexcept;

    VKLoader(const VKLoader &) = delete;
    VKLoader(VKLoader &&) = default;

    ~VKLoader() noexcept;

  private:
    VKLoader() = default;

    std::optional<SymLoader> m_vk_library;
    VKVersion m_version{0, 0};
};
