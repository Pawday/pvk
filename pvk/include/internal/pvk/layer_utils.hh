#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <pvk/logger.hh>

#include "pvk/vk_api.hh"

using LayerExtMap =
    std::unordered_map<std::string, std::unordered_set<std::string>>;

namespace pvk {

void dump_extensions_per_layer(const LayerExtMap &lay_exts, const std::string_view label, Logger &l);
std::unordered_set<std::string> get_instance_layers(Logger &l);
std::unordered_set<std::string>
    get_device_layers(VkPhysicalDevice device, Logger &l);

std::unordered_set<std::string>
    get_instance_layer_extensions(const char *layer_name, Logger &l);

LayerExtMap get_instance_layers_extensions(
    const std::unordered_set<std::string> &layer_names, Logger &l
);

std::unordered_set<std::string> get_device_layer_extensions(
    const VkPhysicalDevice &device, const char *layer_name, Logger &l
);

LayerExtMap get_device_layers_extensions(
    const VkPhysicalDevice &device,
    const std::unordered_set<std::string> &layer_names,
    Logger &l
);

} // namespace pvk
