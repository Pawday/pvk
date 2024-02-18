#include <format>
#include <memory>

#include <pvk/vk_result.hh>

#include <pvk/extensions/debug_utils.hh>

#pragma message "TODO: make proper include path for log.hh"
#include "log.hh"

namespace pvk {

DebugUtilsContext::DebugUtilsContext() noexcept = default;

VkBool32 callback(
    DebugUtilsEXT::MessageSeverityFlagBits messageSeverity,
    DebugUtilsEXT::MessageTypeFlags messageTypes,
    const DebugUtilsEXT::MessengerCallbackData *pCallbackData,
    void *pUserData
)
{
}

std::unique_ptr<DebugUtilsContext> DebugUtilsContext::create(
    VkInstance instance, VkAllocationCallbacks *allocator
) noexcept
{
    std::unique_ptr<DebugUtilsContext> output =
        std::unique_ptr<DebugUtilsContext>(new DebugUtilsContext());
    if (output == nullptr) {
        return nullptr;
    }

    output->m_instance = instance;
    output->m_alloc_callbacks = allocator;

    using MsgTFlag = DebugUtilsEXT::MessageTypeFlagBits;
    DebugUtilsEXT::MessageTypeFlags msgs = 0;
    msgs |= static_cast<VkFlags>(MsgTFlag::GENERAL_BIT);
    msgs |= static_cast<VkFlags>(MsgTFlag::VALIDATION_BIT);
    msgs |= static_cast<VkFlags>(MsgTFlag::PERFORMANCE_BIT);

    using LogLevelFlag = DebugUtilsEXT::MessageSeverityFlagBits;
    DebugUtilsEXT::MessageSeverityFlags log_level = 0;
    log_level |= static_cast<VkFlags>(LogLevelFlag::ERROR_BIT);
    log_level |= static_cast<VkFlags>(LogLevelFlag::WARNING_BIT);
    log_level |= static_cast<VkFlags>(LogLevelFlag::INFO_BIT);
    log_level |= static_cast<VkFlags>(LogLevelFlag::VERBOSE_BIT);

    DebugUtilsEXT::MessengerCreateInfo info{};
    info.pNext = nullptr;
    info.messageType = msgs;
    info.messageSeverity = log_level;
    info.pfnUserCallback = callback;
    info.pUserData = output.get();

    DebugUtilsEXT::Messenger new_messenger = VK_NULL_HANDLE;

    auto create_status = DebugUtilsEXT::CreateMessenger(
        output->m_instance, &info, output->m_alloc_callbacks, &new_messenger
    );
    if (create_status != VK_SUCCESS) {
        log::warning(std::format(
            "DebugUtilsEXT::Messenger - {}", vk_to_str(create_status)
        ));
        return nullptr;
    }

    output->m_debug_messenger = new_messenger;
    return output;
}

DebugUtilsContext::~DebugUtilsContext()
{
    if (m_debug_messenger != VK_NULL_HANDLE) {
        DebugUtilsEXT::DestroyMessenger(
            m_instance, m_debug_messenger, m_alloc_callbacks
        );
        m_instance = VK_NULL_HANDLE;
        m_debug_messenger = VK_NULL_HANDLE;
        m_alloc_callbacks = nullptr;
    }
}

} // namespace pvk
