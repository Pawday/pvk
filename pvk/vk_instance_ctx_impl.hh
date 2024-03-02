#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <stack>
#include <vector>

#include <pvk/vk_allocator.hh>
#include <pvk/vk_instance_ctx.hh>
#include <pvk/logger.hh>

#if PVK_USE_EXT_DEBUG_UTILS
#include <pvk/extensions/debug_utils.hh>
#endif

namespace pvk {
struct alignas(InstanceContext) InstanceContext::Impl
{
    static std::optional<Impl> create();

    static Impl &cast_from(std::byte *data)
    {
        return *reinterpret_cast<Impl *>(data);
    }

    std::vector<VkPhysicalDevice> get_devices() const;

    Impl(Impl &&other) = default;
    Impl &operator=(Impl &&other) = delete;
    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    ~Impl()
    {
        if (m_allocator == nullptr) {
            return;
        }

#if (PVK_USE_EXT_DEBUG_UTILS)
        debugger.reset();
#endif
        vkDestroyInstance(m_vk_instance, m_allocator->get_callbacks());
        m_vk_instance = VK_NULL_HANDLE;

#if (PVK_USE_EXT_DEBUG_UTILS)
        instance_spy.reset();
#endif

    }

  private:

    Logger l;

    static bool check_size()
    {
        static_assert(
            InstanceContext::impl_size >= sizeof(InstanceContext::Impl)
        );
        return InstanceContext::impl_size >= sizeof(InstanceContext::Impl);
    }

    Impl() = default;

    VkInstance m_vk_instance = VK_NULL_HANDLE;
    std::unique_ptr<Allocator> m_allocator = nullptr;

#if (PVK_USE_EXT_DEBUG_UTILS)
    std::unique_ptr<DebugUtilsContext> instance_spy = nullptr;
    std::unique_ptr<DebugUtilsContext> debugger = nullptr;
#endif

    std::stack<VkResult> m_vk_error_stack;
};
} // namespace pvk
