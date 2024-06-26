#pragma once

#include <memory>
#include <new>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <cstddef>
#include <cstdint>

#include <pvk/device.hh>
#include <pvk/instance.hh>
#include <pvk/logger.hh>

#include "pvk/internal/layer_utils.hh"
#include "pvk/internal/vk_allocator.hh"
#include "pvk/internal/vk_api.hh"

#define IMPL Impl::cast_from(*this)

namespace pvk {

struct alignas(Device) Device::Impl
{
    bool connect();
    bool connected() const
    {
        return m_device != VK_NULL_HANDLE;
    }

    void disconnect();

    Impl(VkPhysicalDevice &&device) noexcept;

    ~Impl()
    {
        if (m_phy_device == VK_NULL_HANDLE) {
            return;
        }

        if (m_device == VK_NULL_HANDLE) {
            return;
        }

        disconnect();
    }

    Impl(Impl &&o) noexcept
        : l(std::move(o.l)), m_alloc(std::move(o.m_alloc)),
          m_device_meta(std::move(o.m_device_meta)),
          m_phy_device(o.m_phy_device), m_device(o.m_device),
          m_queues(std::move(o.m_queues))
    {
        if (this == &o) {
            return;
        }
        o.m_phy_device = VK_NULL_HANDLE;
        o.m_device = VK_NULL_HANDLE;
    }

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    static Impl &cast_from(Device &device)
    {
        return *std::launder(reinterpret_cast<Impl *>(device.impl));
    }

    static Impl const &cast_from(Device const &device)
    {
        return *std::launder(reinterpret_cast<Impl const *>(device.impl));
    }

    static bool assert_size()
    {
        static_assert(sizeof(Impl) < Device::impl_size);
        return true;
    }

    VkPhysicalDeviceProperties get_device_props()
    {
        if (std::holds_alternative<VkPhysicalDeviceProperties>(m_device_meta)) {
            return std::get<VkPhysicalDeviceProperties>(m_device_meta);
        }
        VkPhysicalDeviceProperties new_props{};
        vkGetPhysicalDeviceProperties(m_phy_device, &new_props);
        m_device_meta = new_props;
        return new_props;
    }

    VkPhysicalDeviceFeatures get_device_features()
    {
        if (std::holds_alternative<VkPhysicalDeviceFeatures>(m_device_meta)) {
            return std::get<VkPhysicalDeviceFeatures>(m_device_meta);
        }
        VkPhysicalDeviceFeatures new_features{};
        vkGetPhysicalDeviceFeatures(m_phy_device, &new_features);
        m_device_meta = new_features;
        return new_features;
    }

    std::vector<VkQueueFamilyProperties> get_queue_families()
    {
        if (std::holds_alternative<std::vector<VkQueueFamilyProperties>>(
                m_device_meta)) {
            return std::get<std::vector<VkQueueFamilyProperties>>(
                m_device_meta);
        }

        uint32_t nb_queues = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            m_phy_device, &nb_queues, nullptr);

        std::vector<VkQueueFamilyProperties> queues_props(nb_queues);
        vkGetPhysicalDeviceQueueFamilyProperties(
            m_phy_device, &nb_queues, queues_props.data());

        m_device_meta = queues_props;
        return queues_props;
    }

    std::string get_name()
    {
        return get_device_props().deviceName;
    }

    DeviceType get_device_type();

  private:
    Logger l;
    std::unique_ptr<Allocator> m_alloc = nullptr;

    std::variant<
        std::monostate,
        VkPhysicalDeviceProperties,
        VkPhysicalDeviceFeatures,
        std::vector<VkQueueFamilyProperties>>
        m_device_meta;

    VkPhysicalDevice m_phy_device = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

    struct Queue
    {
        using QueueIndex = uint32_t;
        using FamilyIndex = uint32_t;
        VkQueue queue;
        FamilyIndex family_idx;
        QueueIndex idx;
    };

    std::vector<Queue> m_queues;
};

} // namespace pvk
