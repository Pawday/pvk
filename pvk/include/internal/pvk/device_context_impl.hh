#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <cstddef>
#include <cstdint>

#include <pvk/device_context.hh>
#include <pvk/logger.hh>
#include <pvk/physical_device.hh>

#include "pvk/vk_allocator.hh"
#include "pvk/vk_api.hh"

namespace pvk {

struct alignas(DeviceContext) DeviceContext::Impl
{
    static std::optional<DeviceContext>
        create(PhysicalDevice &phy_dev) noexcept;

    bool connect();
    bool connected() const
    {
        return m_device != VK_NULL_HANDLE;
    }

    void disconnect();

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
          m_phy_device(o.m_phy_device), m_device(o.m_device)
    {
        if (this == &o) {
            return;
        }
        o.m_phy_device = VK_NULL_HANDLE;
        o.m_device = VK_NULL_HANDLE;
    }

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    static Impl &cast_from(std::byte *data)
    {
        return *reinterpret_cast<Impl *>(data);
    }

    static Impl const &cast_from(std::byte const *data)
    {
        return *reinterpret_cast<Impl const *>(data);
    }

    static bool assert_size()
    {
        static_assert(sizeof(VkPhysicalDevice) < sizeof(PhysicalDevice));
        static_assert(sizeof(Impl) < DeviceContext::impl_size);
        return true;
    }

    void load_device_props()
    {
        if (std::holds_alternative<VkPhysicalDeviceProperties>(m_device_meta)) {
            return;
        }
        VkPhysicalDeviceProperties new_props{};
        vkGetPhysicalDeviceProperties(m_phy_device, &new_props);
        m_device_meta = new_props;
    }

    void load_device_features()
    {
        if (std::holds_alternative<VkPhysicalDeviceFeatures>(m_device_meta)) {
            return;
        }
        VkPhysicalDeviceFeatures new_features{};
        vkGetPhysicalDeviceFeatures(m_phy_device, &new_features);
        m_device_meta = new_features;
    }

    void load_queue_families()
    {
        if (std::holds_alternative<std::vector<VkQueueFamilyProperties>>(
                m_device_meta)) {
            return;
        }

        uint32_t nb_queues = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            m_phy_device, &nb_queues, nullptr);

        std::vector<VkQueueFamilyProperties> queues_props(nb_queues);
        vkGetPhysicalDeviceQueueFamilyProperties(
            m_phy_device, &nb_queues, queues_props.data());

        m_device_meta = queues_props;
    }

    std::string get_name()
    {
        load_device_props();
        return std::get<VkPhysicalDeviceProperties>(m_device_meta).deviceName;
    }

    DeviceType get_device_type();

  private:
    Impl(VkPhysicalDevice &&device) noexcept;

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
};

} // namespace pvk
