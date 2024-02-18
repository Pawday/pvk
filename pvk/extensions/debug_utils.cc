#include <pvk/extensions/debug_utils.hh>

using namespace pvk;

using Messenger = DebugUtilsEXT::Messenger;
using Label = DebugUtilsEXT::Label;
using MessengerCallbackData = DebugUtilsEXT::MessengerCallbackData;
using ObjectTagInfo = DebugUtilsEXT::ObjectTagInfo;
using MessengerCreateInfo = DebugUtilsEXT::MessengerCreateInfo;
using ObjectNameInfo = DebugUtilsEXT::ObjectNameInfo;

using MessageSeverityFlags = DebugUtilsEXT::MessageSeverityFlags;
using MessageTypeFlags = DebugUtilsEXT::MessageTypeFlags;
using MessengerCallbackDataFlags = DebugUtilsEXT::MessengerCallbackDataFlags;
using MessengerCreateFlags = DebugUtilsEXT::MessengerCreateFlags;

using MessageTypeFlagBits = DebugUtilsEXT::MessageTypeFlagBits;
using MessageSeverityFlagBits = DebugUtilsEXT::MessageSeverityFlagBits;

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

bool DebugUtilsEXT::load(VkInstance instance) noexcept
{
    if (loaded) {
        return true;
    }

    beginLabel = reinterpret_cast<PFN_BeginLabel>(
        vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT")
    );
    endLabel = reinterpret_cast<PFN_EndLabel>(
        vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT")
    );
    insertLabel = reinterpret_cast<PFN_InsertLabel>(
        vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT")
    );
    createMessenger = reinterpret_cast<PFN_CreateMessenger>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
    );
    destroyMessenger = reinterpret_cast<PFN_DestroyMessenger>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );
    queueBeginLabel = reinterpret_cast<PFN_QueueBeginLabel>(
        vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT")
    );
    queueEndLabel = reinterpret_cast<PFN_QueueEndLabel>(
        vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT")
    );
    queueInsertLabel = reinterpret_cast<PFN_QueueInsertLabel>(
        vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT")
    );
    setObjectName = reinterpret_cast<PFN_SetObjectName>(
        vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT")
    );
    setObjectTag = reinterpret_cast<PFN_SetObjectTag>(
        vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT")
    );
    submitMessage = reinterpret_cast<PFN_SubmitMessage>(
        vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT")
    );

    if (some_PFN_is_null()) {
        unload();
        return false;
    }

    loaded = true;
    return true;
}

void DebugUtilsEXT::BeginLabel(
    VkCommandBuffer commandBuffer, const Label *pLabelInfo
)
{
    beginLabel(commandBuffer, pLabelInfo);
}

void DebugUtilsEXT::EndLabel(VkCommandBuffer commandBuffer)
{
    endLabel(commandBuffer);
}

void DebugUtilsEXT::InsertLabel(
    VkCommandBuffer commandBuffer, const Label *pLabelInfo
)
{
    insertLabel(commandBuffer, pLabelInfo);
}

VkResult DebugUtilsEXT::CreateMessenger(
    VkInstance instance,
    const MessengerCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    Messenger *pMessenger
)
{
    return createMessenger(instance, pCreateInfo, pAllocator, pMessenger);
}

void DebugUtilsEXT::DestroyMessenger(
    VkInstance instance,
    Messenger messenger,
    const VkAllocationCallbacks *pAllocator
)
{
    destroyMessenger(instance, messenger, pAllocator);
}

void DebugUtilsEXT::QueueInsertLabel(VkQueue queue, const Label *pLabelInfo)
{
    queueInsertLabel(queue, pLabelInfo);
}

void DebugUtilsEXT::QueueBeginLabel(VkQueue queue, const Label *pLabelInfo)
{
    queueBeginLabel(queue, pLabelInfo);
}

void DebugUtilsEXT::QueueEndLabel(VkQueue queue)
{
    queueEndLabel(queue);
}

VkResult DebugUtilsEXT::SetObjectName(
    VkDevice device, const ObjectNameInfo *pNameInfo
)
{
    return setObjectName(device, pNameInfo);
}

VkResult DebugUtilsEXT::SetObjectTag(
    VkDevice device, const ObjectTagInfo *pTagInfo
)
{
    return setObjectTag(device, pTagInfo);
}

void DebugUtilsEXT::SubmitMessage(
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
