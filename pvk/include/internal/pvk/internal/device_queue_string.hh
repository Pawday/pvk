#pragma once

#include <string>

#include "vk_api.hh"

static inline std::string vk_to_str(VkQueueFlagBits flags)
{
    switch (flags) {
    case VK_QUEUE_FLAG_BITS_MAX_ENUM:
        return "VK_QUEUE_FLAG_MAX_ENUM";
    case VK_QUEUE_GRAPHICS_BIT:
        return "VK_QUEUE_GRAPHICS";
    case VK_QUEUE_COMPUTE_BIT:
        return "VK_QUEUE_COMPUTE";
    case VK_QUEUE_TRANSFER_BIT:
        return "VK_QUEUE_TRANSFER";
    case VK_QUEUE_SPARSE_BINDING_BIT:
        return "VK_QUEUE_SPARSE_BINDING";
    case VK_QUEUE_PROTECTED_BIT:
        return "VK_QUEUE_PROTECTED";
        break;
    }

    return "VK_QUEUE_FLAG_UNKNOWN";
}
