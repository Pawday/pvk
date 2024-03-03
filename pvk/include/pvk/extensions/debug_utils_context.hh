#if !defined(PVK_USE_EXT_DEBUG_UTILS)
#error "Enable PVK_USE_EXT_DEBUG_UTILS if you want to use DebugUtilsContext"
#endif

#pragma once

#include <memory>
#include <utility>


#include <pvk/extensions/debug_utils.hh>
#include <pvk/logger.hh>
#include <pvk/vk_allocator.hh>

namespace pvk {

struct DebugUtilsContext
{
    struct Messenger
    {
        Messenger(
            DebugUtilsEXT::Messenger m,
            VkInstance instance,
            const VkAllocationCallbacks *callbacks
        )
            : m_value(m), m_instance(instance), m_callbacks(callbacks){};

        Messenger(const Messenger &) = delete;
        Messenger &operator=(const Messenger &) = delete;

        Messenger(Messenger &&other)
        {
            m_value = other.m_value;
            other.m_value = VK_NULL_HANDLE;

            m_instance = other.m_instance;
            other.m_instance = VK_NULL_HANDLE;

            m_callbacks = other.m_callbacks;
            other.m_callbacks = nullptr;
        }
        Messenger &operator=(Messenger &&other)
        {
            std::swap(m_value, other.m_value);
            std::swap(m_instance, other.m_instance);
            std::swap(m_callbacks, other.m_callbacks);
            return *this;
        }

        ~Messenger() noexcept;

        const DebugUtilsEXT::Messenger &val() const
        {
            return m_value;
        }

      private:
        DebugUtilsEXT::Messenger m_value = VK_NULL_HANDLE;
        VkInstance m_instance = VK_NULL_HANDLE;
        const VkAllocationCallbacks *m_callbacks = nullptr;
    };

    static std::unique_ptr<DebugUtilsContext>
        create(std::shared_ptr<pvk::Allocator> allocator) noexcept;
    bool attach_to(VkInstanceCreateInfo &instance_info) noexcept;
    bool create_messenger(
        VkInstance instance, const VkAllocationCallbacks *callbacks
    ) noexcept;

    DebugUtilsContext(const DebugUtilsContext &) = delete;
    DebugUtilsContext &operator=(const DebugUtilsContext &) = delete;
    DebugUtilsContext(DebugUtilsContext &&) = delete;
    DebugUtilsContext &operator=(DebugUtilsContext &&) = delete;

    Logger &get_logger() noexcept
    {
        return l;
    }

  private:
    Logger l;
    bool m_instance_spy = false;
    DebugUtilsEXT::MessengerCreateInfo m_info{};
    DebugUtilsContext() noexcept;
    Messenger m_debug_messenger = {VK_NULL_HANDLE, VK_NULL_HANDLE, nullptr};
    std::shared_ptr<pvk::Allocator> m_allocator = nullptr;
};

} // namespace pvk
