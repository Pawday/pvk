#pragma once

#include "pvk/device_context.hh"
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <pvk/instance_context.hh>
#include <pvk/logger.hh>

#if (PVK_USE_EXT_DEBUG_UTILS)
#include <pvk/extensions/debug_utils_context.hh>
#include <string>
#endif

#include "pvk/vk_allocator.hh"
#include "pvk/vk_api.hh"

namespace pvk {
struct alignas(InstanceContext) InstanceContext::Impl
{
    static std::optional<InstanceContext> create();

    static void assert_size_for_move()
    {
        static_assert(sizeof(InstanceContext::Impl) <= impl_size);
    }
    Impl(Impl &&other) noexcept;
    Impl &operator=(Impl &&other) = delete;
    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    ~Impl() noexcept;

    static Impl &cast_from(InstanceContext &inst)
    {
        return *reinterpret_cast<Impl *>(inst.impl);
    }

    static Impl const &cast_from(InstanceContext const &inst)
    {
        return *reinterpret_cast<Impl const *>(inst.impl);
    }

    bool load_devices();

    size_t get_device_count() const noexcept;
    std::optional<DeviceContext> get_device(size_t device_idx) const noexcept;

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

    static bool check_impl_sizes()
    {
        static_assert(
            sizeof(InstanceContext::Impl) < InstanceContext::impl_size);

        return true;
    }

    Impl() = default;
};

} // namespace pvk
