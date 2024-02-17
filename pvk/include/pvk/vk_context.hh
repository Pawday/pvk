#pragma once

#include <optional>
#include <stack>

#include <cstddef>

#include <pvk/symvis.hh>

#include "vk_api.hh"

namespace pvk {

struct PVK_API alignas(std::max_align_t) Context
{
    static std::optional<Context> create() noexcept;
    Context(Context &&) noexcept;
    Context &operator=(Context &&) noexcept;
    ~Context();

    Context(const Context &) = delete;
    Context &operator=(const Context &) = delete;

    std::stack<VkResult> get_error_stack() const;

  private:
    Context();
    static constexpr size_t impl_size = 512;
    std::byte impl[impl_size];

    struct Impl;
};
} // namespace pvk
