#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <pvk/symvis.hh>

#include "dso_loader.hh"

namespace pvk {
struct PVK_API Loader
{
    struct VKVersion
    {
        using Major_t = uint8_t;
        using Minor_t = uint16_t;
        Major_t major;
        Minor_t minor;
    };

    using Optional = std::optional<Loader>;
    static Optional load(const std::string &library) noexcept;

    VKVersion get_version() const
    {
        return m_version;
    }

    Loader(const Loader &) = delete;
    Loader(Loader &&) = default;

    ~Loader() noexcept;

  private:
    Loader() = default;

    std::optional<SymLoader> m_vk_library;
    VKVersion m_version{0, 0};
};
} // namespace pvk
