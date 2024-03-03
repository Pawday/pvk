#pragma once

#include <optional>
#include <vector>

#include <cstddef>

#include <pvk/symvis.hh>
#include <pvk/physical_device.hh>

namespace pvk {

struct PVK_API alignas(std::max_align_t) InstanceContext
{
    static std::optional<InstanceContext> create() noexcept;
    InstanceContext(InstanceContext &&) noexcept;
    InstanceContext &operator=(InstanceContext &&) noexcept;
    ~InstanceContext() noexcept;

    InstanceContext(const InstanceContext &) = delete;
    InstanceContext &operator=(const InstanceContext &) = delete;

    std::vector<PhysicalDevice> get_devices() const;

  private:
    static constexpr size_t impl_size = 128;
    std::byte impl[impl_size];
    struct Impl;
    InstanceContext(Impl &&) noexcept;
};
} // namespace pvk
