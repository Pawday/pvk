#include <algorithm>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cstdint>

#include <pvk/logger.hh>

#include "pvk/internal/layer_utils.hh"
#include "pvk/internal/log_utils.hh"
#include "pvk/internal/result.hh"
#include "pvk/internal/vk_api.hh"

namespace pvk {

void dump_extensions_per_layer(
    const LayerExtMap &lay_exts, std::string_view label, Logger &l)
{
    if (lay_exts.size() == 0) {
        return;
    }

    size_t max_line_size = label.size() + 2;
    std::vector<std::string> lines;
    lines.reserve(lay_exts.size());

    bool first = true;
    for (auto &layer : lay_exts) {

        if (first) {
            first = false;
        } else {
            lines.emplace_back();
        }

        lines.emplace_back(std::format("{}", std::string(layer.first)));

        for (auto &extension : layer.second) {
            lines.emplace_back(std::format("- {}", std::string(extension)));
        }
        if (layer.second.size() == 0) {
            lines.emplace_back(std::format(" {}", "(No extensions)"));
        }
    }

    auto max_line =
        std::ranges::max_element(lines, [](const auto &lhs, const auto &rhs) {
            return lhs.size() < rhs.size();
        });
    if (max_line != std::end(lines) && max_line->size() > max_line_size) {
        max_line_size = max_line->size();
    }

    l.info("{}", box_title(label, max_line_size));
    for (auto &line : lines) {
        l.info("{}", box_entry(line, max_line_size));
    }
    l.info("{}", box_foot(max_line_size));
}

namespace {
std::unordered_set<std::string>
    collect_layers(const std::vector<VkLayerProperties> &layers, Logger &l)
{
    std::unordered_set<std::string> output;
    output.reserve(layers.size());
    for (auto &layer : layers) {
        std::string layer_name(layer.layerName);
        if (output.contains(layer_name)) {
            l.warning("Layer {} mentioned more than once", layer_name);
            continue;
        }
        output.emplace(layer_name);
    }

    return output;
};

std::unordered_set<std::string> collect_extensions(
    const std::vector<VkExtensionProperties> &extensions, Logger &l)
{
    std::unordered_set<std::string> output;
    output.reserve(extensions.size());
    for (auto &ext : extensions) {
        std::string ext_name(ext.extensionName);
        if (output.contains(ext_name)) {
            std::string dup_warn = std::format(
                "Extention \"{}\" mentioned more than once", ext_name);
            l.warning("{}", dup_warn);
            continue;
        }
        output.emplace(std::move(ext_name));
    }
    return output;
}

} // namespace

std::unordered_set<std::string> get_instance_layers(Logger &l)
{
    uint32_t nb_layers = 0;
    VkResult get_nb_layers_status =
        vkEnumerateInstanceLayerProperties(&nb_layers, nullptr);

    if (VK_SUCCESS != get_nb_layers_status) {
        l.warning(
            "Cannot retreave instance layer count ({}): Assume no layers",
            vk_to_str(get_nb_layers_status));
        return {};
    }

    std::vector<VkLayerProperties> available_layers(nb_layers);
    VkResult get_layers_status =
        vkEnumerateInstanceLayerProperties(&nb_layers, available_layers.data());

    if (VK_SUCCESS != get_layers_status) {
        l.warning(
            "Cannot retreave instance layers ({}): Assume no layers",
            vk_to_str(get_layers_status));
        return {};
    }

    return collect_layers(available_layers, l);
}

std::unordered_set<std::string>
    get_device_layers(VkPhysicalDevice device, Logger &l)
{
    uint32_t nb_layers = 0;
    VkResult get_nb_layers_status =
        vkEnumerateDeviceLayerProperties(device, &nb_layers, nullptr);

    if (VK_SUCCESS != get_nb_layers_status) {
        l.warning(
            "Cannot retreave device layers count ({}): Assume no layers",
            vk_to_str(get_nb_layers_status));
        return {};
    }

    std::vector<VkLayerProperties> available_layers(nb_layers);
    VkResult get_layers_status = vkEnumerateDeviceLayerProperties(
        device, &nb_layers, available_layers.data());

    if (VK_SUCCESS != get_layers_status) {
        l.warning(
            "Cannot retreave device layers ({}): Assume no layers",
            vk_to_str(get_nb_layers_status));
        return {};
    }

    return collect_layers(available_layers, l);
}

std::unordered_set<std::string>
    get_instance_layer_extensions(const char *layer_name, Logger &l)
{
    uint32_t nb_extensions = 0;
    VkResult get_nb_status = vkEnumerateInstanceExtensionProperties(
        layer_name, &nb_extensions, nullptr);

    if (VK_SUCCESS != get_nb_status) {
        l.warning(
            "Cannot retreave instance layers extension count ({}): Assume no "
            "instance layers",
            vk_to_str(get_nb_status));
        return {};
    }

    std::vector<VkExtensionProperties> layer_exts(nb_extensions);
    VkResult get_layers_status = vkEnumerateInstanceExtensionProperties(
        layer_name, &nb_extensions, layer_exts.data());

    if (VK_SUCCESS != get_layers_status) {
        l.warning(
            "Cannot retreave instance layer extensions ({}): Assume no "
            "instance "
            "layers",
            vk_to_str(get_layers_status));
        return {};
    }

    return collect_extensions(layer_exts, l);
}

LayerExtMap get_instance_layers_extensions(
    const std::unordered_set<std::string> &layer_names, Logger &l)
{
    LayerExtMap output;
    for (const auto &layer_name : layer_names) {
        std::unordered_set<std::string> layer_extensions =
            get_instance_layer_extensions(layer_name.c_str(), l);

        auto layer_extensions_list = output.find(layer_name);

        if (layer_extensions_list == std::end(output)) {
            std::unordered_set<std::string> new_extension_list;
            new_extension_list.reserve(layer_extensions.size());
            output.emplace(layer_name, std::move(new_extension_list));

            layer_extensions_list = output.find(layer_name);
            if (layer_extensions_list == std::end(output)) {
                l.error("No memory for layers extension list");
                continue;
            }
        }

        for (auto &extension : layer_extensions) {
            layer_extensions_list->second.emplace(std::move(extension));
        }
    }
    return output;
}

std::unordered_set<std::string> get_device_layer_extensions(
    const VkPhysicalDevice &device, const char *layer_name, Logger &l)
{
    uint32_t nb_extensions = 0;
    VkResult get_nb_status = vkEnumerateDeviceExtensionProperties(
        device, layer_name, &nb_extensions, nullptr);

    if (VK_SUCCESS != get_nb_status) {
        l.warning(
            "Cannot retreave device layers extension count ({}): Assume no "
            "device layers",
            vk_to_str(get_nb_status));
        return {};
    }

    std::vector<VkExtensionProperties> layer_exts(nb_extensions);
    VkResult get_layers_status = vkEnumerateDeviceExtensionProperties(
        device, layer_name, &nb_extensions, layer_exts.data());

    if (VK_SUCCESS != get_layers_status) {
        l.warning(
            "Cannot retreave device layer extensions ({}): Assume no "
            "device layers",
            vk_to_str(get_layers_status));
        return {};
    }

    return collect_extensions(layer_exts, l);
}

LayerExtMap get_device_layers_extensions(
    const VkPhysicalDevice &device,
    const std::unordered_set<std::string> &layer_names,
    Logger &l)
{
    LayerExtMap output;
    for (const auto &layer_name : layer_names) {
        std::unordered_set<std::string> layer_extensions =
            get_device_layer_extensions(device, layer_name.c_str(), l);

        auto layer_extensions_list = output.find(layer_name);

        if (layer_extensions_list == std::end(output)) {
            std::unordered_set<std::string> new_extension_list;
            new_extension_list.reserve(layer_extensions.size());
            output.emplace(layer_name, std::move(new_extension_list));

            layer_extensions_list = output.find(layer_name);
            if (layer_extensions_list == std::end(output)) {
                l.error("No memory for layers extension list");
                continue;
            }
        }

        for (auto &extension : layer_extensions) {
            layer_extensions_list->second.emplace(std::move(extension));
        }
    }
    return output;
}

} // namespace pvk
