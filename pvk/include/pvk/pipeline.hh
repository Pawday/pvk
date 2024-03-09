#pragma once

#include <cstddef>

#include <memory>

#include <optional>
#include <pvk/device_context.hh>
#include <pvk/symvis.hh>

namespace pvk {

struct PVK_API alignas(std::max_align_t) Pipeline
{
    static std::optional<Pipeline>
        create(std::shared_ptr<DeviceContext> device_context) noexcept;
    Pipeline(Pipeline &&) noexcept;
    ~Pipeline() noexcept;

    Pipeline(const Pipeline &) = delete;

    Pipeline &operator=(const Pipeline &) = delete;
    Pipeline &operator=(Pipeline &&) = delete;

  private:
    static constexpr size_t impl_size = 128;
    std::byte impl[impl_size];
    struct Impl;
    Pipeline(Impl &&) noexcept;
};

} // namespace pvk
