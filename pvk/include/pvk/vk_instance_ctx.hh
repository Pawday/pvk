#pragma once

#include <optional>

#include <cstddef>

#include <pvk/symvis.hh>

namespace pvk {

struct PVK_API alignas(std::max_align_t) InstanceContext
{
    static std::optional<InstanceContext> create() noexcept;
    InstanceContext(InstanceContext &&) noexcept;
    InstanceContext &operator=(InstanceContext &&) noexcept;
    ~InstanceContext();

    InstanceContext(const InstanceContext &) = delete;
    InstanceContext &operator=(const InstanceContext &) = delete;


  private:
    InstanceContext() = default;
    static constexpr size_t impl_size = 128;
    std::byte impl[impl_size];
    struct Impl;
};
} // namespace pvk
