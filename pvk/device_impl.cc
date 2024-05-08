#include <algorithm>
#include <bitset>
#include <format>
#include <iterator>
#include <limits>
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
#include "pvk/device_queue_string.hh"
#include "pvk/layer_utils.hh"
#include "pvk/log.hh"
#include "pvk/log_utils.hh"
#include "pvk/result.hh"
#include "pvk/string_pack.hh"
#include "pvk/vk_api.hh"

constexpr float DEFAULT_QUEUE_PRIORITY = 1.0;

#define IMPL Impl::cast_from(*this)

namespace pvk {

bool Device::connect()
{
    return IMPL.connect();
}

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
        l.notice("Layer \"VK_LAYER_KHRONOS_validation\" is not supported");
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

        std::ranges::copy(full_extesions_list, std::back_inserter(lines));
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

    auto enabled_ext_names =
        utils::StringPack::create(std::span(enabled_extensions));
    if (!enabled_layer_names || !enabled_ext_names) {
        l.warning(
            "Device connection failue: No host memory for connection info");
        return false;
    }
    auto enabled_layers_names_ptrs = enabled_layer_names->get();
    auto enabled_extension_names_ptrs = enabled_ext_names->get();

    std::vector<VkQueueFamilyProperties> families = get_queue_families();
    if (families.size() == 0) {
        l.warning("No single queue family (WAT?)");
    }

    VkDeviceQueueCreateInfo empty_q_create_info{};
    empty_q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    std::vector<VkDeviceQueueCreateInfo> q_create_infos;
    q_create_infos.resize(families.size());

    std::vector<std::vector<float>> q_create_infos_priors_storage;
    q_create_infos_priors_storage.resize(families.size());

    for (size_t q_fam_idx = 0; q_fam_idx < families.size(); q_fam_idx++) {
        auto &family = families[q_fam_idx];
        if (family.queueCount == 0) {
            l.warning("No single queue in Queue family #{}", q_fam_idx);
            continue;
        }

        VkQueueFlags raw_flags = family.queueFlags;

        std::string queue_idx_title = std::format(
            "Queue family #{} ({} {})",
            q_fam_idx,
            family.queueCount,
            family.queueCount == 1 ? "queue" : "queues");
        std::string queue_flags_str = "Raw flags: 0b" +
            std::bitset<std::numeric_limits<VkQueueFlags>::digits>(raw_flags)
                .to_string();

        auto extract_flags =
            [](decltype(raw_flags) flags) -> std::vector<std::string> {
            std::vector<std::string> output;

            constexpr auto bits = std::numeric_limits<decltype(flags)>::digits;
            for (size_t bit_idx = 0; bit_idx < bits; bit_idx++) {
                size_t bit_mask = 1;
                bit_mask <<= bit_idx;
                VkQueueFlagBits bit =
                    static_cast<VkQueueFlagBits>(flags & bit_mask);
                if (bit == 0) {
                    continue;
                }
                output.emplace_back(
                    std::format("bit {} - {}", bit_idx, vk_to_str(bit)));
            }
            return output;
        };

        size_t max_string =
            std::max(queue_flags_str.size(), queue_idx_title.size());

        std::vector<std::string> flags_strlist = extract_flags(raw_flags);

        auto max_flag_str = std::ranges::max_element(
            flags_strlist,
            [](const auto &l, const auto &r) { return l.size() < r.size(); });
        if (std::end(flags_strlist) != max_flag_str) {
            max_string = std::max(max_string, max_flag_str->size());
        }

        l.info("{}", box_title(queue_idx_title, max_string));
        l.info("{}", box_entry(queue_flags_str, max_string));
        for (auto &flag_str : flags_strlist) {
            l.info("{}", box_entry(flag_str, max_string));
        }
        l.info("{}", box_foot(max_string));

        std::vector<std::string> q_info_box;
    }

    std::ranges::fill(q_create_infos, empty_q_create_info);
    for (size_t q_fam_idx = 0; q_fam_idx < families.size(); q_fam_idx++) {
        VkQueueFamilyProperties &q_fam_props = families[q_fam_idx];
        VkDeviceQueueCreateInfo &q_create_info = q_create_infos[q_fam_idx];
        q_create_info.queueFamilyIndex = q_fam_idx;
        q_create_info.queueCount = q_fam_props.queueCount;
        q_create_info.flags = 0;
        auto &q_priors = q_create_infos_priors_storage[q_fam_idx];
        q_priors.resize(q_create_info.queueCount);
        std::ranges::fill(q_priors, DEFAULT_QUEUE_PRIORITY);
        q_create_info.pQueuePriorities = q_priors.data();
    }

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    VkPhysicalDeviceFeatures device_features{};
    create_info.pEnabledFeatures = &device_features;
    create_info.pQueueCreateInfos = q_create_infos.data();
    create_info.queueCreateInfoCount = q_create_infos.size();
    create_info.ppEnabledLayerNames = enabled_layers_names_ptrs.data();
    create_info.enabledLayerCount = enabled_layers_names_ptrs.size();
    create_info.enabledExtensionCount = enabled_extension_names_ptrs.size();
    create_info.ppEnabledExtensionNames = enabled_extension_names_ptrs.data();

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

DeviceType Device::get_device_type()
{
    return IMPL.get_device_type();
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
    IMPL.~Impl();
}

std::string Device::get_name()
{
    return IMPL.get_name();
}

bool Device::connected() const
{
    return IMPL.connected();
}

} // namespace pvk
