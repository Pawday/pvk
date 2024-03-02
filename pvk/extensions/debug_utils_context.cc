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
    pvk::debug(std::format("[VkDebugUtils] {}", pCallbackData->pMessage));
    return VK_FALSE;
}

std::unique_ptr<DebugUtilsContext>
    DebugUtilsContext::create(const VkAllocationCallbacks *allocator) noexcept
{
    std::unique_ptr<DebugUtilsContext> output =
        std::unique_ptr<DebugUtilsContext>(new DebugUtilsContext());
    if (output == nullptr) {
        return nullptr;
    }

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

    output->m_info.pNext = nullptr;
    output->m_info.messageType = msgs;
    output->m_info.messageSeverity = log_level;
    output->m_info.pfnUserCallback = callback;
    output->m_info.pUserData = output.get();

    return output;
}

bool DebugUtilsContext::attach_to(VkInstanceCreateInfo &instance_info) noexcept
{
    if (m_debug_messenger.val() != VK_NULL_HANDLE) {
        return false;
    }

    if (m_instance_spy) {
        pvk::debug("DebugUtilsContext already attached to instance_info");
        return false;
    }

    VkBaseOutStructure *info_addr =
        reinterpret_cast<VkBaseOutStructure *>(&m_info);

    VkBaseOutStructure *cur_node =
        reinterpret_cast<VkBaseOutStructure *>(&instance_info);
    const VkBaseOutStructure *next = cur_node->pNext;

    while (next != nullptr) {
        if (next == info_addr) {
            pvk::warning("DebugUtilsContext already attached"
                         "to instance_info somehow");
            m_instance_spy = true;
            return true;
        }

        cur_node = cur_node->pNext;
        next = cur_node->pNext;
    }

    cur_node->pNext = info_addr;

    m_instance_spy = true;
    return true;
}

DebugUtilsContext::Messenger::~Messenger() noexcept
{
    if (m_value != VK_NULL_HANDLE) {
        DebugUtilsEXT::DestroyMessenger(m_instance, m_value, m_callbacks);
    }
}

bool DebugUtilsContext::create_messenger(
    VkInstance instance, const VkAllocationCallbacks *callbacks
) noexcept
{
    if (m_debug_messenger.val() != VK_NULL_HANDLE) {
        return false;
    }

    if (instance == VK_NULL_HANDLE) {
        return false;
    }

    if (m_instance_spy) {
        return false;
    }

    DebugUtilsEXT::Messenger new_messenger = VK_NULL_HANDLE;
    auto create_status = DebugUtilsEXT::CreateMessenger(
        instance, &m_info, m_alloc_callbacks, &new_messenger
    );
    if (create_status != VK_SUCCESS) {
        pvk::warning(
            std::format("Messager create failue - {}", vk_to_str(create_status))
        );
        return false;
    }

    m_debug_messenger = Messenger(new_messenger, instance, callbacks);

    return true;
}

} // namespace pvk
