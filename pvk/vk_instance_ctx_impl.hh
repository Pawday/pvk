#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <stack>

#include <pvk/vk_allocator.hh>
#include <pvk/vk_context.hh>

#if PVK_USE_EXT_DEBUG_UTILS
#include <pvk/extensions/debug_utils.hh>
#endif

namespace pvk {
struct alignas(Context) Context::Impl
{
    static std::optional<Impl> create();

    Impl(Impl &&other) = default;

    Impl &operator=(Impl &&other) = default;

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    static bool chech_size()
    {
        static_assert(Context::impl_size >= sizeof(Context::Impl));
        return Context::impl_size >= sizeof(Context::Impl);
    }

    bool check_phy_device(VkPhysicalDevice dev)
    {
        VkPhysicalDeviceFeatures dev_features;
        vkGetPhysicalDeviceFeatures(dev, &dev_features);

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);

        return true;
    };

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

    static Impl &cast_from(std::byte *data)
    {
        return *reinterpret_cast<Impl *>(data);
    }

  private:
    Impl() = default;

    VkInstance m_vk_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_vk_device = VK_NULL_HANDLE;

    std::unique_ptr<Allocator> m_allocator = nullptr;

#if (PVK_USE_EXT_DEBUG_UTILS)
    std::unique_ptr<DebugUtilsContext> instance_spy = nullptr;
    std::unique_ptr<DebugUtilsContext> debugger = nullptr;
#endif

    std::stack<VkResult> m_vk_error_stack;
};
} // namespace pvk
