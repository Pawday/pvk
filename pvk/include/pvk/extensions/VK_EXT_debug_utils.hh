#if !defined(PVK_USE_EXT_DEBUG_UTILS)
#error "Enable PVK_USE_EXT_DEBUG_UTILS if you want to use it"
#endif

#pragma once

#include <cstddef>
#include <cstdint>

#include <vk_platform.h>

#include <pvk/vk_api.hh>

#define VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT 1000128000
#define VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT 1000128001
#define VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT 1000128002
#define VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT 1000128003
#define VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT 1000128004

#define VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT 1000128000

namespace pvk {

struct DebugUtilsEXT
{
    VK_DEFINE_NON_DISPATCHABLE_HANDLE(Messenger)

    /* +---- Enums ----+ */
    enum class MessageSeverityFlagBits
    {
        VERBOSE_BIT = 0x00000001,
        INFO_BIT = 0x00000010,
        WARNING_BIT = 0x00000100,
        ERROR_BIT = 0x00001000,
    };

    enum class MessageTypeFlagBits
    {
        GENERAL_BIT = 0x00000001,
        VALIDATION_BIT = 0x00000002,
        PERFORMANCE_BIT = 0x00000004,
        // VK_EXT_device_address_binding_report
        DEVICE_ADDRESS_BINDING_BIT = 0x00000008,
    };

    /* +---- Bitmasks ----+ */
    typedef VkFlags MessageSeverityFlags;
    typedef VkFlags MessageTypeFlags;
    typedef VkFlags MessengerCallbackDataFlags;
    typedef VkFlags MessengerCreateFlags;

    /* +---- Structures ----+ */
    struct Label;
    struct MessengerCallbackData;
    struct ObjectTagInfo;
    struct MessengerCreateInfo;
    struct ObjectNameInfo;

    /* +---- Function Pointers ----+ */
    typedef VkBool32(VKAPI_PTR *PFN_vkDebugUtilsMessengerCallback)(
        MessageSeverityFlagBits messageSeverity,
        MessageTypeFlags messageTypes,
        const MessengerCallbackData *pCallbackData,
        void *pUserData
    );

    class Label
    {
        VkStructureType sType =
            static_cast<VkStructureType>(VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT
            );

      public:
        const void *pNext;
        const char *pLabelName;
        float color[4];
    };

    class MessengerCallbackData
    {
        VkStructureType sType = static_cast<VkStructureType>(
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT
        );

      public:
        const void *pNext;
        MessengerCallbackDataFlags flags;
        const char *pMessageIdName;
        int32_t messageIdNumber;
        const char *pMessage;
        uint32_t queueLabelCount;
        const Label *pQueueLabels;
        uint32_t cmdBufLabelCount;
        const Label *pCmdBufLabels;
        uint32_t objectCount;
        const ObjectNameInfo *pObjects;
    };

    class ObjectTagInfo
    {
        VkStructureType sType = static_cast<VkStructureType>(
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT
        );

      public:
        const void *pNext;
        VkObjectType objectType;
        uint64_t objectHandle;
        uint64_t tagName;
        size_t tagSize;
        const void *pTag;
    };

    class MessengerCreateInfo
    {
        VkStructureType sType = static_cast<VkStructureType>(
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
        );

      public:
        const void *pNext;

      private:
        MessengerCreateFlags flags = 0;

      public:
        MessageSeverityFlags messageSeverity;
        MessageTypeFlags messageType;
        PFN_vkDebugUtilsMessengerCallback pfnUserCallback;
        void *pUserData;
    };

    class ObjectNameInfo
    {
        VkStructureType sType = static_cast<VkStructureType>(
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT
        );

      public:
        VkObjectType objectType;
        uint64_t objectHandle;
        const char *pObjectName;
    };

    static bool load(VkInstance instance) noexcept;

    void BeginLabel(VkCommandBuffer commandBuffer, const Label *pLabelInfo);
    void EndLabel(VkCommandBuffer commandBuffer);
    void InsertLabel(VkCommandBuffer commandBuffer, const Label *pLabelInfo);

    VkResult CreateMessenger(
        VkInstance instance,
        const MessengerCreateInfo *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        Messenger *pMessenger
    );

    void DestroyMessenger(
        VkInstance instance,
        Messenger messenger,
        const VkAllocationCallbacks *pAllocator
    );

    void QueueInsertLabel(VkQueue queue, const Label *pLabelInfo);
    void QueueBeginLabel(VkQueue queue, const Label *pLabelInfo);
    void QueueEndLabel(VkQueue queue);

    VkResult SetObjectName(VkDevice device, const ObjectNameInfo *pNameInfo);

    VkResult SetObjectTag(VkDevice device, const ObjectTagInfo *pTagInfo);

    void SubmitMessage(
        VkInstance instance,
        MessageSeverityFlagBits messageSeverity,
        MessageTypeFlags messageTypes,
        const MessengerCallbackData *pCallbackData
    );
};

} // namespace pvk
