#pragma once

#include <string>

#include "vk_api.hh"

static inline std::string vk_to_str(VkQueueFlagBits flags)
{

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFlagBits.html
    enum class UpstreamVkQueueFlagBits
    {
        GRAPHICS = 0x00000001,
        COMPUTE = 0x00000002,
        TRANSFER = 0x00000004,
        SPARSE_BINDING = 0x00000008,
        // Provided by VK_VERSION_1_1
        PROTECTED = 0x00000010,
        // Provided by VK_KHR_video_decode_queue
        VIDEO_DECODE_KHR = 0x00000020,
        // Provided by VK_KHR_video_encode_queue
        VIDEO_ENCODE_KHR = 0x00000040,
        // Provided by VK_NV_optical_flow
        OPTICAL_FLOW_NV = 0x00000100,
    };

    UpstreamVkQueueFlagBits upstream_flags =
        static_cast<UpstreamVkQueueFlagBits>(flags);

    switch (upstream_flags) {
    case UpstreamVkQueueFlagBits::GRAPHICS:
        return "GRAPHICS";
    case UpstreamVkQueueFlagBits::COMPUTE:
        return "COMPUTE";
    case UpstreamVkQueueFlagBits::TRANSFER:
        return "TRANSFER";
    case UpstreamVkQueueFlagBits::SPARSE_BINDING:
        return "SPARSE_BINDING";
    case UpstreamVkQueueFlagBits::PROTECTED:
        return "PROTECTED";
    case UpstreamVkQueueFlagBits::VIDEO_DECODE_KHR:
        return "VIDEO_DECODE_KHR";
    case UpstreamVkQueueFlagBits::VIDEO_ENCODE_KHR:
        return "VIDEO_ENCODE_KHR";
    case UpstreamVkQueueFlagBits::OPTICAL_FLOW_NV:
        return "OPTICAL_FLOW_NV";
    }

    return "VK_QUEUE_FLAG_UNKNOWN_BIT";
}
