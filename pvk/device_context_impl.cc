#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include <pvk/logger.hh>
#include <pvk/physical_device.hh>
#include <pvk/vk_device_ctx.hh>
#include <pvk/vk_instance_ctx.hh>
#include <variant>
#include <vector>

#include "pvk/phy_device_conv.hh"
#include "pvk/vk_api.hh"

namespace pvk {

struct alignas(DeviceContext) DeviceContext::Impl
{
    static std::optional<DeviceContext> create(PhysicalDevice &phy_dev) noexcept
    {
        auto native = as_native(phy_dev);
        if (native == VK_NULL_HANDLE) {
            return std::nullopt;
        }

        Impl impl_out(std::move(native));
        DeviceContext out(std::move(impl_out));
        return out;
    }

    Impl(Impl &&o) noexcept
        : l(std::move(o.l)), device_meta(o.device_meta),
          m_phy_device(o.m_phy_device)
    {
    }

    void load_device_props()
    {
        if (std::holds_alternative<VkPhysicalDeviceProperties>(device_meta)) {
            return;
        }
        VkPhysicalDeviceProperties new_props;
        vkGetPhysicalDeviceProperties(m_phy_device, &new_props);
        device_meta = new_props;
    }

    void load_device_features()
    {
        if (std::holds_alternative<VkPhysicalDeviceFeatures>(device_meta)) {
            return;
        }
        VkPhysicalDeviceFeatures new_features;
        vkGetPhysicalDeviceFeatures(m_phy_device, &new_features);
        device_meta = new_features;
    }

    void load_queue_families()
    {
        if (std::holds_alternative<std::vector<VkQueueFamilyProperties>>(
                device_meta
            )) {
            return;
        }

        uint32_t nb_queues = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            m_phy_device, &nb_queues, nullptr
        );

        std::vector<VkQueueFamilyProperties> queues_props(nb_queues);
        vkGetPhysicalDeviceQueueFamilyProperties(
            m_phy_device, &nb_queues, queues_props.data()
        );

        device_meta = queues_props;
    }

    std::string get_name()
    {
        load_device_props();
        return std::get<VkPhysicalDeviceProperties>(device_meta).deviceName;
    }

    DeviceType get_device_type()
    {
        load_device_props();
        auto vk_device_type =
            std::get<VkPhysicalDeviceProperties>(device_meta).deviceType;

        switch (vk_device_type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            return DeviceType::UNKNOWN;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return DeviceType::GPU;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return DeviceType::CPU;
        }
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
        static_assert(sizeof(Impl) < DeviceContext::impl_size);
        return true;
    }

  private:
    Impl(VkPhysicalDevice &&device);

    Logger l;

    std::variant<
        std::monostate,
        VkPhysicalDeviceProperties,
        VkPhysicalDeviceFeatures,
        std::vector<VkQueueFamilyProperties>>
        device_meta;

    VkPhysicalDevice m_phy_device = VK_NULL_HANDLE;
};

DeviceContext::Impl::Impl(VkPhysicalDevice &&device)
    : m_phy_device(std::move(device))
{
}

std::optional<DeviceContext> DeviceContext::create(PhysicalDevice &device
) noexcept
{
    return Impl::create(device);
}

DeviceContext::DeviceContext(Impl &&o) noexcept
{
    new (impl) Impl(std::move(o));
}

DeviceContext::DeviceContext(DeviceContext &&o) noexcept
{
    Impl &other_impl = Impl::cast_from(o.impl);
    new (impl) Impl(std::move(other_impl));
}

DeviceContext &DeviceContext::operator=(DeviceContext &&o) noexcept
{
    std::swap(this->impl, o.impl);
    return *this;
}

DeviceContext::~DeviceContext() noexcept
{
    Impl::cast_from(impl).~Impl();
}

std::string DeviceContext::get_name()
{
    return Impl::cast_from(impl).get_name();
}

DeviceType DeviceContext::get_device_type()
{
    return Impl::cast_from(impl).get_device_type();
}

} // namespace pvk
