#pragma once

#include <cstddef>
#include <memory>
#include <optional>

#include <pvk/vk_instance_ctx.hh>

namespace pvk {

struct DeviceContext
{
    std::optional<DeviceContext>
        create(std::shared_ptr<InstanceContext> instance) noexcept;

    DeviceContext(DeviceContext &&) noexcept;
    DeviceContext &operator=(DeviceContext &&) noexcept;

    DeviceContext(const DeviceContext &) = delete;
    DeviceContext &operator=(const DeviceContext &) = delete;

  private:
    DeviceContext() = default;
    static constexpr size_t impl_size = 128;
    std::byte impl[impl_size];
    struct Impl;
};

} // namespace pvk
