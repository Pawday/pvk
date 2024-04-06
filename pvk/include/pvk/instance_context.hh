#pragma once

#include <optional>

#include <cstddef>

#include <pvk/device_context.hh>
#include <pvk/symvis.hh>

namespace pvk {

struct PVK_API alignas(std::max_align_t) InstanceContext
{
    static std::optional<InstanceContext> create() noexcept;
    InstanceContext(InstanceContext &&) noexcept;
    InstanceContext &operator=(InstanceContext &&) noexcept;
    ~InstanceContext() noexcept;

    InstanceContext(const InstanceContext &) = delete;
    InstanceContext &operator=(const InstanceContext &) = delete;

    size_t get_device_count() const noexcept;
    std::optional<DeviceContext> get_device(size_t device_idx) const noexcept;

  private:
    static constexpr size_t impl_size = 128;
    std::byte impl[impl_size];
    struct Impl;
    InstanceContext(Impl &&) noexcept;
};
} // namespace pvk
