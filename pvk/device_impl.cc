#include <algorithm>
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

#include <pvk/device.hh>
#include <pvk/instance.hh>
#include <pvk/logger.hh>
#include <pvk/vk_allocator.hh>

#include "pvk/device_impl.hh"
#include "pvk/layer_utils.hh"
#include "pvk/log_utils.hh"
#include "pvk/result.hh"
#include "pvk/string_pack.hh"
#include "pvk/vk_api.hh"

namespace pvk {

bool Device::Impl::connect()
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
        std::string label = std::format("{} extensions", device_name);
        size_t max_line_size = label.size();
        std::vector<std::string> lines;
        lines.reserve(full_extesions_list.size());

        for (auto &ext : full_extesions_list) {
            lines.emplace_back(ext);
        }

        std::ranges::sort(lines);

        auto max_line = std::ranges::max_element(
            lines, [](const auto &lhs, const auto &rhs) {
                return lhs.size() < rhs.size();
            });
        if (max_line != std::end(lines) && max_line_size < max_line->size()) {
            max_line_size = max_line->size();
        }

        l.info("{}", box_title(label, max_line_size));
        for (auto &line : lines) {
            l.info("{}", box_entry(line, max_line_size));
        }
        l.info("{}", box_foot(max_line_size));
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

#pragma message "Resolve temp"
    utils::StringPack::create(std::span(enabled_layers))->get();

    std::vector<VkQueueFamilyProperties> families = get_queue_families();
    if (families.size() == 0) {
        l.warning("No single queue family to configure from");
        return false;
    }

    size_t q_idx = 0;
    size_t nb_queues = 1;
    std::vector<float> priors;
    priors.resize(nb_queues);
    std::ranges::fill(priors, 1.0f);

    VkDeviceQueueCreateInfo main_queue{};
    main_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    main_queue.queueCount = nb_queues;
    main_queue.queueFamilyIndex = q_idx;
    main_queue.pQueuePriorities = priors.data();

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
        l.warning(
            "Device connection failue: {}", vk_to_str(create_device_status));
        return false;
    }

    m_device = new_logical_device;

    return true;
}

void Device::Impl::disconnect()
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

DeviceType Device::Impl::get_device_type()
{
    auto vk_device_type = get_device_props().deviceType;

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
        l.debug("Vk GPU type: {}", gpu_type);
        return DeviceType::GPU;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return DeviceType::CPU;
    }

    l.error("Unhandled vKDeviceType: Assume UNKNOWN");
    return DeviceType::UNKNOWN;
}

Device::Impl::Impl(VkPhysicalDevice &&device) noexcept
    : m_phy_device(std::move(device))
{
    l.set_name(get_name());
    m_alloc = std::make_unique<Allocator>();
}

Device::Device(Impl &&o) noexcept
{
    new (impl) Impl(std::move(o));
}

Device::Device(Device &&o) noexcept
{
    new (impl) Impl(std::move(Impl::cast_from(o)));
}

Device &Device::operator=(Device &&o) noexcept
{
    std::swap(this->impl, o.impl);
    return *this;
}

Device::~Device() noexcept
{
    Impl::cast_from(*this).~Impl();
}

std::string Device::get_name()
{
    return Impl::cast_from(*this).get_name();
}

DeviceType Device::get_device_type()
{
    return Impl::cast_from(*this).get_device_type();
}

bool Device::connect()
{
    return Impl::cast_from(*this).connect();
}

bool Device::connected() const
{
    return Impl::cast_from(*this).connected();
}

} // namespace pvk
