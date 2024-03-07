#include <cstddef>
#include <cstring>
#include <format>
#include <optional>
#include <string>
#include <utility>

#include <pvk/logger.hh>
#include <pvk/physical_device.hh>
#include <pvk/vk_device_ctx.hh>
#include <pvk/vk_instance_ctx.hh>
#include <variant>

#include "pvk/log.hh"
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
        : l(std::move(o.l)), device_meta(o.device_meta), m_phy_device(o.m_phy_device)
    {
    }

    std::string get_name()
    {
        if (!std::holds_alternative<VkPhysicalDeviceProperties>(device_meta)) {
            VkPhysicalDeviceProperties new_props;
            vkGetPhysicalDeviceProperties(m_phy_device, &new_props);
            device_meta = new_props;
        }

        return std::get<VkPhysicalDeviceProperties>(device_meta).deviceName;
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

    ~Impl()
    {
    }

  private:
    Impl(VkPhysicalDevice &&device);

    Logger l;

    std::variant<VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures> device_meta;


    VkPhysicalDevice m_phy_device = VK_NULL_HANDLE;
};

DeviceContext::Impl::Impl(VkPhysicalDevice &&device)
    : m_phy_device(std::move(device))
{
    VkPhysicalDeviceProperties new_props;
    vkGetPhysicalDeviceProperties(m_phy_device, &new_props);
    device_meta = new_props;
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

} // namespace pvk
