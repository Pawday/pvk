#pragma once

#include <optional>
#include <string>

#include <cstddef>

#include <pvk/physical_device.hh>
#include <pvk/vk_instance_ctx.hh>

#include <pvk/symvis.hh>

namespace pvk {

struct PVK_API alignas(std::max_align_t) DeviceContext
{
    static std::optional<DeviceContext> create(PhysicalDevice &device
    ) noexcept;
    DeviceContext(DeviceContext &&) noexcept;
    DeviceContext &operator=(DeviceContext &&) noexcept;

    ~DeviceContext() noexcept;

    std::string get_name();

    DeviceContext(DeviceContext const &) = delete;
    DeviceContext &operator=(DeviceContext const &) = delete;

  private:
    static constexpr size_t impl_size = 1024;
    std::byte impl[impl_size];
    struct Impl;
    DeviceContext(Impl &&) noexcept;
};

} // namespace pvk
