#pragma once

#include <optional>
#include <string>

#include <cstddef>

#include <pvk/symvis.hh>

namespace pvk {

enum class DeviceType
{
    UNKNOWN,
    GPU,
    CPU
};

struct PVK_API alignas(std::max_align_t) DeviceContext
{
    DeviceContext(DeviceContext &&) noexcept;
    DeviceContext &operator=(DeviceContext &&) noexcept;

    ~DeviceContext() noexcept;

    std::string get_name();
    DeviceType get_device_type();

    bool connect();
    bool connected() const;

    DeviceContext(DeviceContext const &) = delete;
    DeviceContext &operator=(DeviceContext const &) = delete;

    struct Impl;
    DeviceContext(Impl &&) noexcept;

  private:
    static constexpr size_t impl_size = 1024;
    std::byte impl[impl_size];
};

} // namespace pvk
