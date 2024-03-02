#pragma once

#include <optional>
#include <stack>

#include <cstddef>

#include <pvk/symvis.hh>

#include "vk_api.hh"

namespace pvk {

struct PVK_API alignas(std::max_align_t) InstanceContext
{
    static std::optional<InstanceContext> create() noexcept;
    InstanceContext(InstanceContext &&) noexcept;
    InstanceContext &operator=(InstanceContext &&) noexcept;
    ~InstanceContext();

    InstanceContext(const InstanceContext &) = delete;
    InstanceContext &operator=(const InstanceContext &) = delete;

    std::stack<VkResult> get_error_stack() const;

  private:
    InstanceContext() = default;
    static constexpr size_t impl_size = 128;
    std::byte impl[impl_size];

    struct Impl;
};
} // namespace pvk
