#include <optional>
#include <utility>
#if !defined(PVK_USE_EXT_DEBUG_UTILS)
#error "Enable PVK_USE_EXT_DEBUG_UTILS if you want to use it"
#endif

#pragma once

#include <memory>

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
    class Label;
    class MessengerCallbackData;
    class ObjectTagInfo;
    class MessengerCreateInfo;
    class ObjectNameInfo;

    /* +---- Function Pointers ----+ */
    typedef VkBool32(VKAPI_PTR *PFN_vkDebugUtilsMessengerCallback)(
        MessageSeverityFlagBits messageSeverity,
        MessageTypeFlags messageTypes,
        const MessengerCallbackData *pCallbackData,
        void *pUserData
    );

    class Label
    {
        [[maybe_unused]] VkStructureType sType =
            static_cast<VkStructureType>(VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT
            );

      public:
        const void *pNext;
        const char *pLabelName;
        float color[4];
    };

    class MessengerCallbackData
    {
        [[maybe_unused]] VkStructureType sType = static_cast<VkStructureType>(
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
        [[maybe_unused]] VkStructureType sType = static_cast<VkStructureType>(
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
        [[maybe_unused]] VkStructureType sType = static_cast<VkStructureType>(
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
        );

      public:
        const void *pNext;

      private:
        [[maybe_unused]] MessengerCreateFlags flags =
            0; // Vulkan 1.3 reserved field

      public:
        MessageSeverityFlags messageSeverity;
        MessageTypeFlags messageType;
        PFN_vkDebugUtilsMessengerCallback pfnUserCallback;
        void *pUserData;
    };

    class ObjectNameInfo
    {
        [[maybe_unused]] VkStructureType sType = static_cast<VkStructureType>(
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT
        );

      public:
        VkObjectType objectType;
        uint64_t objectHandle;
        const char *pObjectName;
    };

    static bool load(VkInstance instance) noexcept;

    static void
        BeginLabel(VkCommandBuffer commandBuffer, const Label *pLabelInfo);
    static void EndLabel(VkCommandBuffer commandBuffer);
    static void
        InsertLabel(VkCommandBuffer commandBuffer, const Label *pLabelInfo);

    static VkResult CreateMessenger(
        VkInstance instance,
        const MessengerCreateInfo *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        Messenger *pMessenger
    );

    static void DestroyMessenger(
        VkInstance instance,
        Messenger messenger,
        const VkAllocationCallbacks *pAllocator
    );

    static void QueueInsertLabel(VkQueue queue, const Label *pLabelInfo);
    static void QueueBeginLabel(VkQueue queue, const Label *pLabelInfo);
    static void QueueEndLabel(VkQueue queue);

    static VkResult
        SetObjectName(VkDevice device, const ObjectNameInfo *pNameInfo);

    static VkResult
        SetObjectTag(VkDevice device, const ObjectTagInfo *pTagInfo);

    static void SubmitMessage(
        VkInstance instance,
        MessageSeverityFlagBits messageSeverity,
        MessageTypeFlags messageTypes,
        const MessengerCallbackData *pCallbackData
    );
};

struct DebugUtilsContext
{
    struct Messenger
    {
        Messenger(
            DebugUtilsEXT::Messenger m,
            VkInstance instance,
            const VkAllocationCallbacks *callbacks
        )
            : m_value(m), m_instance(instance), m_callbacks(callbacks){};

        Messenger(const Messenger &) = delete;
        Messenger &operator=(const Messenger &) = delete;

        Messenger(Messenger &&other)
        {
            m_value = other.m_value;
            other.m_value = VK_NULL_HANDLE;

            m_instance = other.m_instance;
            other.m_instance = VK_NULL_HANDLE;

            m_callbacks = other.m_callbacks;
            other.m_callbacks = nullptr;
        }
        Messenger &operator=(Messenger &&other)
        {
            std::swap(m_value, other.m_value);
            std::swap(m_instance, other.m_instance);
            std::swap(m_callbacks, other.m_callbacks);
            return *this;
        }

        ~Messenger() noexcept;

        const DebugUtilsEXT::Messenger &val() const
        {
            return m_value;
        }

      private:
        DebugUtilsEXT::Messenger m_value = VK_NULL_HANDLE;
        VkInstance m_instance = VK_NULL_HANDLE;
        const VkAllocationCallbacks *m_callbacks = nullptr;
    };

    static std::unique_ptr<DebugUtilsContext>
        create(const VkAllocationCallbacks *allocator) noexcept;
    bool attach_to(VkInstanceCreateInfo &instance_info) noexcept;
    bool create_messenger(
        VkInstance instance, const VkAllocationCallbacks *callbacks
    ) noexcept;

    DebugUtilsContext(const DebugUtilsContext &) = delete;
    DebugUtilsContext &operator=(const DebugUtilsContext &) = delete;
    DebugUtilsContext(DebugUtilsContext &&) = delete;
    DebugUtilsContext &operator=(DebugUtilsContext &&) = delete;

  private:
    bool m_instance_spy = false;
    DebugUtilsEXT::MessengerCreateInfo m_info{};
    DebugUtilsContext() noexcept;
    Messenger m_debug_messenger = {VK_NULL_HANDLE, VK_NULL_HANDLE, nullptr};
    const VkAllocationCallbacks *m_alloc_callbacks = nullptr;
};

} // namespace pvk
