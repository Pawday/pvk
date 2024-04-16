#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <optional>
#include <string_view>
#include <vector>

#include <pvk/device.hh>
#include <pvk/instance.hh>
#include <pvk/logger.hh>

#if (PVK_USE_EXT_DEBUG_UTILS)
#include <pvk/extensions/debug_utils_context.hh>
#endif

#include "pvk/vk_allocator.hh"
#include "pvk/vk_api.hh"

namespace pvk {
struct alignas(Instance) Instance::Impl
{
    static std::optional<Instance> create();

    Impl(Impl &&other) noexcept;
    Impl &operator=(Impl &&other) = delete;
    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    ~Impl() noexcept;

    static Impl &cast_from(Instance &inst)
    {
        return *std::launder(reinterpret_cast<Impl *>(inst.impl));
    }

    static Impl const &cast_from(Instance const &inst)
    {
        return *std::launder(reinterpret_cast<Impl const *>(inst.impl));
    }

    bool load_devices();

    size_t get_device_count() const noexcept;
    std::optional<Device> get_device(size_t device_idx) const noexcept;

  private:
    std::shared_ptr<Allocator> m_allocator = nullptr;
    VkInstance m_vk_instance = VK_NULL_HANDLE;
    Logger l;
    std::vector<VkPhysicalDevice> m_devices;

#if (PVK_USE_EXT_DEBUG_UTILS)
    static void debug_utils_log_cb(
        void *user_data,
        Logger::Level level,
        const std::string_view &message) noexcept;
    std::unique_ptr<DebugUtilsContext> instance_spy = nullptr;
    std::unique_ptr<DebugUtilsContext> debugger = nullptr;
#endif

    static void check_impl_sizes()
    {
        static_assert(sizeof(Instance::Impl) < Instance::impl_size);
    }

    Impl() = default;
};

} // namespace pvk
