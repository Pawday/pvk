#include <format>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cstddef>

#include <pvk/device_context.hh>
#include <pvk/instance_context.hh>
#include <pvk/logger.hh>
#include <pvk/physical_device.hh>
#include <pvk/vk_allocator.hh>

#include "pvk/device_context_impl.hh"
#include "pvk/layer_utils.hh"
#include "pvk/phy_device_conv.hh"
#include "pvk/result.hh"
#include "pvk/string_pack.hh"
#include "pvk/vk_api.hh"

namespace pvk {

std::optional<DeviceContext>
    DeviceContext::Impl::create(PhysicalDevice &phy_dev) noexcept
{
    auto native = as_native(phy_dev);
    if (native == VK_NULL_HANDLE) {
        return std::nullopt;
    }

    Impl impl_out(std::move(native));
    DeviceContext out(std::move(impl_out));
    return out;
}

bool DeviceContext::Impl::connect()
{
    if (m_device != VK_NULL_HANDLE) {
        l.warning("Connection already established: Ignore connect request");
        return false;
    }

    std::unordered_set<std::string> full_extesions_list;
    std::vector<std::string_view> enabled_layers;
    std::vector<std::string_view> enabled_extensions;

    auto implicit_extensions =
        get_device_layer_extensions(m_phy_device, nullptr, l);
    for (auto &implicit_extension : implicit_extensions) {
        full_extesions_list.emplace(std::move(implicit_extension));
    }

    std::unordered_set<std::string> device_layers =
        get_device_layers(m_phy_device, l);
    auto device_ext_map =
        get_device_layers_extensions(m_phy_device, device_layers, l);

    for (auto &layer : device_ext_map) {
        for (auto &layer_ext_name : layer.second) {
            full_extesions_list.emplace(std::move(layer_ext_name));
        }
    }

#if defined(PVK_USE_KHR_VALIDATION_LAYER)
    if (device_layers.contains("VK_LAYER_KHRONOS_validation")) {
        l.info("Layer \"VK_LAYER_KHRONOS_validation\" is enabled");
        enabled_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    } else {
        l.warning("Layer \"VK_LAYER_KHRONOS_validation\" is not supported");
    }
#endif

    std::string device_name = get_name();
    dump_extensions_per_layer(
        device_ext_map, std::format("{} layers", device_name), l);

    if (full_extesions_list.size() != 0) {
        l.info(std::format(
            "+{:=^50}+", std::format(" {} extensions ", device_name)));
        for (auto &ext : full_extesions_list) {
            l.info(std::format("| {: <49}|", ext));
        }
        l.info(std::format("+{:=^50}+", ""));
    }

    auto enabled_layer_names =
        utils::StringPack::create(std::span(enabled_layers));
    auto enabled_layers_names_ptrs = enabled_layer_names->get();
    auto enabled_ext_names =
        utils::StringPack::create(std::span(enabled_extensions));
    if (!enabled_layer_names || !enabled_ext_names) {
        l.warning(
            "Device connection failue: No host memory for connection info");
        return false;
    }

    load_queue_families();

    std::vector<VkQueueFamilyProperties> families =
        std::get<std::vector<VkQueueFamilyProperties>>(m_device_meta);

    if (families.size() == 0) {
        l.warning("No single queue family to configure from");
        return false;
    }

    VkDeviceQueueCreateInfo main_queue{};
    main_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    main_queue.queueCount = 1;
    main_queue.queueFamilyIndex = 0;
    float queue_priority = 1.0;
    main_queue.pQueuePriorities = &queue_priority;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    VkPhysicalDeviceFeatures device_features{};
    create_info.pEnabledFeatures = &device_features;
    create_info.pQueueCreateInfos = &main_queue;
    create_info.queueCreateInfoCount = 1;
    create_info.ppEnabledLayerNames = enabled_layers_names_ptrs.data();
    create_info.enabledLayerCount = enabled_layers_names_ptrs.size();

    VkDevice new_logical_device = VK_NULL_HANDLE;
    VkResult create_device_status = vkCreateDevice(
        m_phy_device,
        &create_info,
        m_alloc->get_callbacks(),
        &new_logical_device);

    if (VK_SUCCESS != create_device_status) {
        l.warning(std::format(
            "Device connection failue: {}", vk_to_str(create_device_status)));
        return false;
    }

    m_device = new_logical_device;

    return true;
}

void DeviceContext::Impl::disconnect()
{
    if (m_device == VK_NULL_HANDLE) {
        l.warning("No connection: Ignore disconnect request");
        return;
    }

    if (m_alloc != nullptr) {
        vkDestroyDevice(m_device, m_alloc->get_callbacks());
    } else {
        vkDestroyDevice(m_device, nullptr);
    }
    m_device = VK_NULL_HANDLE;
}

DeviceType DeviceContext::Impl::get_device_type()
{
    load_device_props();
    auto vk_device_type =
        std::get<VkPhysicalDeviceProperties>(m_device_meta).deviceType;

    std::string gpu_type;

    switch (vk_device_type) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        gpu_type = "Integrated";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        gpu_type = "Discrete";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        gpu_type = "Virtual";
        break;
    default:
        break;
    }

    switch (vk_device_type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
        return DeviceType::UNKNOWN;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        l.debug(std::format("Vk GPU type: {}", gpu_type));
        return DeviceType::GPU;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return DeviceType::CPU;
    }
}

DeviceContext::Impl::Impl(VkPhysicalDevice &&device) noexcept
    : m_phy_device(std::move(device))
{
    l.set_name(std::format(
        "DeviceContext 0x{:x}", reinterpret_cast<size_t>(m_phy_device)));
    m_alloc = std::make_unique<Allocator>();
}

std::optional<DeviceContext>
    DeviceContext::create(PhysicalDevice &device) noexcept
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

bool DeviceContext::connect()
{
    return Impl::cast_from(impl).connect();
}

bool DeviceContext::connected() const
{
    return Impl::cast_from(impl).connected();
}

} // namespace pvk
