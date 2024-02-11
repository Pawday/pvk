#include "VK_EXT_debug_utils.hh"

using Messenger = VkDebugUtilsEXT::Messenger;
using Label = VkDebugUtilsEXT::Label;
using MessengerCallbackData = VkDebugUtilsEXT::MessengerCallbackData;
using ObjectTagInfo = VkDebugUtilsEXT::ObjectTagInfo;
using MessengerCreateInfo = VkDebugUtilsEXT::MessengerCreateInfo;
using ObjectNameInfo = VkDebugUtilsEXT::ObjectNameInfo;

using MessageSeverityFlags = VkDebugUtilsEXT::MessageSeverityFlags;
using MessageTypeFlags = VkDebugUtilsEXT::MessageTypeFlags;
using MessengerCallbackDataFlags = VkDebugUtilsEXT::MessengerCallbackDataFlags;
using MessengerCreateFlags = VkDebugUtilsEXT::MessengerCreateFlags;

using MessageTypeFlagBits = VkDebugUtilsEXT::MessageTypeFlagBits;
using MessageSeverityFlagBits = VkDebugUtilsEXT::MessageSeverityFlagBits;

using PFN_BeginLabel = void (*)(VkCommandBuffer, const Label *);
using PFN_EndLabel = void (*)(VkCommandBuffer);
using PFN_InsertLabel = void (*)(VkCommandBuffer, const Label *);
using PFN_CreateMessenger =
    VkResult (*)(VkInstance, const MessengerCreateInfo *, const VkAllocationCallbacks *, Messenger *);
using PFN_DestroyMessenger =
    void (*)(VkInstance, Messenger, const VkAllocationCallbacks *);
using PFN_QueueBeginLabel = void (*)(VkQueue, const Label *);
using PFN_QueueEndLabel = void (*)(VkQueue);
using PFN_QueueInsertLabel = void (*)(VkQueue, const Label *);
using PFN_SetObjectName = VkResult (*)(VkDevice, const ObjectNameInfo *);
using PFN_SetObjectTag = VkResult (*)(VkDevice, const ObjectTagInfo *);
using PFN_SubmitMessage =
    void (*)(VkInstance, MessageSeverityFlagBits, MessageTypeFlags, const MessengerCallbackData *);

namespace {
static bool loaded = false;

PFN_BeginLabel beginLabel = nullptr;
PFN_EndLabel endLabel = nullptr;
PFN_InsertLabel insertLabel = nullptr;
PFN_CreateMessenger createMessenger = nullptr;
PFN_DestroyMessenger destroyMessenger = nullptr;
PFN_QueueBeginLabel queueBeginLabel = nullptr;
PFN_QueueEndLabel queueEndLabel = nullptr;
PFN_QueueInsertLabel queueInsertLabel = nullptr;
PFN_SetObjectName setObjectName = nullptr;
PFN_SetObjectTag setObjectTag = nullptr;
PFN_SubmitMessage submitMessage = nullptr;

static void unload()
{
    beginLabel = nullptr;
    endLabel = nullptr;
    insertLabel = nullptr;
    createMessenger = nullptr;
    destroyMessenger = nullptr;
    queueBeginLabel = nullptr;
    queueEndLabel = nullptr;
    queueInsertLabel = nullptr;
    setObjectName = nullptr;
    setObjectTag = nullptr;
    submitMessage = nullptr;
    loaded = false;
}

static bool some_PFN_is_null()
{
    bool some_null = false;
    some_null |= beginLabel == nullptr;
    some_null |= endLabel == nullptr;
    some_null |= insertLabel == nullptr;
    some_null |= createMessenger == nullptr;
    some_null |= destroyMessenger == nullptr;
    some_null |= queueBeginLabel == nullptr;
    some_null |= queueEndLabel == nullptr;
    some_null |= queueInsertLabel == nullptr;
    some_null |= setObjectName == nullptr;
    some_null |= setObjectTag == nullptr;
    some_null |= submitMessage == nullptr;
    return some_null;
}

} // namespace

bool VkDebugUtilsEXT::load() noexcept
{
    if (loaded) {
        return true;
    }

    beginLabel = reinterpret_cast<PFN_BeginLabel>(
        vkGetInstanceProcAddr(nullptr, "vkCmdBeginDebugUtilsLabelEXT")
    );
    endLabel = reinterpret_cast<PFN_EndLabel>(
        vkGetInstanceProcAddr(nullptr, "vkCmdEndDebugUtilsLabelEXT")
    );
    insertLabel = reinterpret_cast<PFN_InsertLabel>(
        vkGetInstanceProcAddr(nullptr, "vkCmdInsertDebugUtilsLabelEXT")
    );
    createMessenger = reinterpret_cast<PFN_CreateMessenger>(
        vkGetInstanceProcAddr(nullptr, "vkCreateDebugUtilsMessengerEXT")
    );
    destroyMessenger = reinterpret_cast<PFN_DestroyMessenger>(
        vkGetInstanceProcAddr(nullptr, "vkDestroyDebugUtilsMessengerEXT")
    );
    queueBeginLabel = reinterpret_cast<PFN_QueueBeginLabel>(
        vkGetInstanceProcAddr(nullptr, "vkQueueBeginDebugUtilsLabelEXT")
    );
    queueEndLabel = reinterpret_cast<PFN_QueueEndLabel>(
        vkGetInstanceProcAddr(nullptr, "vkQueueEndDebugUtilsLabelEXT")
    );
    queueInsertLabel = reinterpret_cast<PFN_QueueInsertLabel>(
        vkGetInstanceProcAddr(nullptr, "vkQueueInsertDebugUtilsLabelEXT")
    );
    setObjectName = reinterpret_cast<PFN_SetObjectName>(
        vkGetInstanceProcAddr(nullptr, "vkSetDebugUtilsObjectNameEXT")
    );
    setObjectTag = reinterpret_cast<PFN_SetObjectTag>(
        vkGetInstanceProcAddr(nullptr, "vkSetDebugUtilsObjectTagEXT")
    );
    submitMessage = reinterpret_cast<PFN_SubmitMessage>(
        vkGetInstanceProcAddr(nullptr, "vkSubmitDebugUtilsMessageEXT")
    );

    if (some_PFN_is_null()) {
        unload();
        return false;
    }

    loaded = true;
    return true;
}

void VkDebugUtilsEXT::BeginLabel(
    VkCommandBuffer commandBuffer, const Label *pLabelInfo
)
{
    beginLabel(commandBuffer, pLabelInfo);
}

void VkDebugUtilsEXT::EndLabel(VkCommandBuffer commandBuffer)
{
    endLabel(commandBuffer);
}

void VkDebugUtilsEXT::InsertLabel(
    VkCommandBuffer commandBuffer, const Label *pLabelInfo
)
{
    insertLabel(commandBuffer, pLabelInfo);
}

VkResult VkDebugUtilsEXT::CreateMessenger(
    VkInstance instance,
    const MessengerCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    Messenger *pMessenger
)
{
    return createMessenger(instance, pCreateInfo, pAllocator, pMessenger);
}

void VkDebugUtilsEXT::DestroyMessenger(
    VkInstance instance,
    Messenger messenger,
    const VkAllocationCallbacks *pAllocator
)
{
    destroyMessenger(instance, messenger, pAllocator);
}

void VkDebugUtilsEXT::QueueInsertLabel(VkQueue queue, const Label *pLabelInfo)
{
    queueInsertLabel(queue, pLabelInfo);
}

void VkDebugUtilsEXT::QueueBeginLabel(VkQueue queue, const Label *pLabelInfo)
{
    queueBeginLabel(queue, pLabelInfo);
}

void VkDebugUtilsEXT::QueueEndLabel(VkQueue queue)
{
    queueEndLabel(queue);
}

VkResult VkDebugUtilsEXT::SetObjectName(
    VkDevice device, const ObjectNameInfo *pNameInfo
)
{
    return setObjectName(device, pNameInfo);
}

VkResult VkDebugUtilsEXT::SetObjectTag(
    VkDevice device, const ObjectTagInfo *pTagInfo
)
{
    return setObjectTag(device, pTagInfo);
}

void VkDebugUtilsEXT::SubmitMessage(
    VkInstance instance,
    MessageSeverityFlagBits messageSeverity,
    MessageTypeFlags messageTypes,
    const MessengerCallbackData *pCallbackData
)
{
    return submitMessage(
        instance, messageSeverity, messageTypes, pCallbackData
    );
}
