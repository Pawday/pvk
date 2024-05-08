#pragma once

#include <string>

#include <cstddef>

#include <pvk/symvis.hh>

namespace pvk {

struct Instance;

enum class DeviceType
{
    UNKNOWN,
    GPU,
    CPU
};

struct PVK_API alignas(std::max_align_t) Device
{
    Device(Device &&) noexcept;
    Device &operator=(Device &&) noexcept;

    ~Device() noexcept;

    std::string get_name();
    DeviceType get_device_type();

    bool connect();
    bool connected() const;

    Device(Device const &) = delete;
    Device &operator=(Device const &) = delete;

  private:
    static constexpr size_t impl_size = 1024;
    std::byte impl[impl_size];
    struct Impl;
    Device(Impl &&) noexcept;

    friend struct Instance;
};

} // namespace pvk
