#pragma once

#include <cstring>

#include <pvk/physical_device.hh>

#include "pvk/vk_api.hh"

namespace pvk {

static inline VkPhysicalDevice as_native(PhysicalDevice &h)
{
    VkPhysicalDevice output = VK_NULL_HANDLE;
    std::memcpy(&output, h.data, sizeof(VkPhysicalDevice));
    return output;
};

static inline PhysicalDevice from_native(VkPhysicalDevice h)
{
    PhysicalDevice output{};
    std::memcpy(output.data, &h, sizeof(VkPhysicalDevice));
    return output;
};

} // namespace pvk
