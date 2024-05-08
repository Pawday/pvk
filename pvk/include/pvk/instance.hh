#pragma once

#include <optional>

#include <cstddef>

#include <pvk/symvis.hh>

#include <pvk/device.hh>

namespace pvk {

struct PVK_API alignas(std::max_align_t) Instance
{
    static std::optional<Instance> create() noexcept;
    Instance(Instance &&) noexcept;
    Instance &operator=(Instance &&) noexcept;
    ~Instance() noexcept;

    Instance(const Instance &) = delete;
    Instance &operator=(const Instance &) = delete;

    size_t get_device_count() const noexcept;
    std::optional<Device> get_device(size_t device_idx) const noexcept;

  private:
    static constexpr size_t impl_size = 256;
    std::byte impl[impl_size];
    struct Impl;
    Instance(Impl &&) noexcept;
};
} // namespace pvk
