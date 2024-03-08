#include <algorithm>
#include <format>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cstddef>
#include <cstdint>

#include <pvk/log.hh>
#include <pvk/logger.hh>
#include <pvk/physical_device.hh>
#include <pvk/vk_allocator.hh>
#include <pvk/vk_api.hh>
#include <pvk/vk_instance_ctx.hh>

#if defined(PVK_USE_EXT_DEBUG_UTILS)
#include <pvk/extensions/debug_utils.hh>
#include <pvk/extensions/debug_utils_context.hh>
#endif

#include "pvk/layer_utils.hh"
#include "pvk/phy_device_conv.hh"
#include "pvk/result.hh"
#include "pvk/string_pack.hh"

namespace pvk {
struct alignas(InstanceContext) InstanceContext::Impl
{
    static std::optional<InstanceContext> create();
    std::vector<VkPhysicalDevice> get_devices() const;

    Impl(Impl &&other) noexcept;
    Impl &operator=(Impl &&other) = delete;
    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    ~Impl() noexcept;

    static Impl &cast_from(std::byte *data)
    {
        return *reinterpret_cast<Impl *>(data);
    }

    static Impl const &cast_from(std::byte const *data)
    {
        return *reinterpret_cast<Impl const *>(data);
    }

  private:
    std::shared_ptr<Allocator> m_allocator = nullptr;
    VkInstance m_vk_instance = VK_NULL_HANDLE;
    Logger l;

#if (PVK_USE_EXT_DEBUG_UTILS)
    static void debug_utils_log_cb(
        void *user_data, Logger::Level level, const std::string &message
    ) noexcept;
    std::unique_ptr<DebugUtilsContext> instance_spy = nullptr;
    std::unique_ptr<DebugUtilsContext> debugger = nullptr;
#endif

    static bool check_impl_sizes()
    {
        static_assert(sizeof(PhysicalDevice) >= sizeof(VkPhysicalDevice));

        if (sizeof(VkPhysicalDevice) > sizeof(PhysicalDevice)) {
            return false;
        }

        static_assert(
            InstanceContext::impl_size >= sizeof(InstanceContext::Impl)
        );
        if (InstanceContext::impl_size < sizeof(InstanceContext::Impl)) {
            return false;
        }

        return true;
    }

    Impl() = default;
};

InstanceContext::InstanceContext(InstanceContext::Impl &&impl) noexcept
{
    new (this->impl) InstanceContext::Impl(std::move(impl));
}

InstanceContext &InstanceContext::operator=(InstanceContext &&other) noexcept
{
    std::swap(this->impl, other.impl);
    return *this;
}

InstanceContext::~InstanceContext() noexcept
{
    Impl::cast_from(impl).~Impl();
}

InstanceContext::Impl::~Impl() noexcept
{
    if (m_vk_instance == NULL) {
        return;
    }

#if (PVK_USE_EXT_DEBUG_UTILS)
    debugger.reset();
#endif
    vkDestroyInstance(m_vk_instance, m_allocator->get_callbacks());
    m_vk_instance = VK_NULL_HANDLE;

#if (PVK_USE_EXT_DEBUG_UTILS)
    instance_spy.reset();
#endif
}

std::optional<InstanceContext> InstanceContext::create() noexcept
{
    return InstanceContext::Impl::create();
}

std::optional<InstanceContext> InstanceContext::Impl::create()
{
    InstanceContext::Impl impl;
    impl.m_allocator = std::make_unique<Allocator>();
    impl.l.set_name("InstanceContext");

    Logger &l = impl.l;

    std::unordered_set<std::string> full_extesions_list;
    std::vector<std::string_view> enabled_layers;
    std::vector<std::string_view> enabled_extensions;

    auto implicit_extensions = get_instance_layer_extensions(nullptr, l);
    for (auto &implicit_extension : implicit_extensions) {
        full_extesions_list.emplace(std::move(implicit_extension));
    }

    std::unordered_set<std::string> vk_layers = get_instance_layers(l);
    LayerExtMap layer_extensions = get_instance_layers_extensions(vk_layers, l);

    for (auto &layer : layer_extensions) {
        for (auto &layer_ext_name : layer.second) {
            full_extesions_list.emplace(std::move(layer_ext_name));
        }
    }

    dump_extensions_per_layer(layer_extensions, "Instance Layers", l);

#if defined(PVK_USE_KHR_VALIDATION_LAYER)
    if (vk_layers.contains("VK_LAYER_KHRONOS_validation")) {
        l.info("Layer \"VK_LAYER_KHRONOS_validation\" is enabled");
        enabled_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    } else {
        l.warning("Layer \"VK_LAYER_KHRONOS_validation\" is not supported");
    }
#endif

    if (full_extesions_list.size() != 0) {
        l.info(std::format("+{:=^50}+", " All instance extensions "));
        for (auto &ext : full_extesions_list) {
            l.info(std::format("| {: <49}|", ext));
        }
        l.info(std::format("+{:=^50}+", ""));
    }

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    bool has_debug_utils = full_extesions_list.contains("VK_EXT_debug_utils");
    bool debug_utils_load_status = false;
    if (has_debug_utils) {
        enabled_extensions.emplace_back("VK_EXT_debug_utils");
    } else {
        l.warning("VK_EXT_debug_utils extension is not supported: Ignore");
    }

    if (has_debug_utils) {
        impl.instance_spy = DebugUtilsContext::create(impl.m_allocator);
        impl.instance_spy->get_logger().set_name("SpyLog");
        impl.instance_spy->get_logger().set_userdata(&impl);
        impl.instance_spy->get_logger().set_callback(debug_utils_log_cb);
        impl.debugger = DebugUtilsContext::create(impl.m_allocator);
        impl.debugger->get_logger().set_name("DebugUtils");
        impl.debugger->get_logger().set_userdata(&impl);
        impl.debugger->get_logger().set_callback(debug_utils_log_cb);
    }

    if (has_debug_utils && impl.instance_spy == nullptr) {
        l.warning("Creating early debug messenger failue");
    }

    if (has_debug_utils && impl.debugger == nullptr) {
        l.warning("Creating primary debug messenger failue");
    }
#endif

    auto enabled_layer_names =
        utils::StringPack::create(std::span(enabled_layers));
    auto enabled_ext_names =
        utils::StringPack::create(std::span(enabled_extensions));
    if (!enabled_layer_names || !enabled_ext_names) {
        return std::nullopt;
    }

    auto en_layer_names_ptrs = enabled_layer_names->get();
    auto en_ext_names_ptrs = enabled_ext_names->get();

    VkApplicationInfo vk_app_info{};
    VkInstanceCreateInfo vk_instance_info{};
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pApplicationName = "Some name";
    vk_app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    vk_instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_instance_info.pApplicationInfo = &vk_app_info;
    vk_instance_info.enabledLayerCount = en_layer_names_ptrs.size();
    vk_instance_info.ppEnabledLayerNames = en_layer_names_ptrs.data();
    vk_instance_info.enabledExtensionCount = en_ext_names_ptrs.size();
    vk_instance_info.ppEnabledExtensionNames = en_ext_names_ptrs.data();

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    bool instance_debug_attach_status = false;

    if (has_debug_utils && impl.instance_spy != nullptr) {
        instance_debug_attach_status =
            impl.instance_spy->attach_to(vk_instance_info);
    }

    if (has_debug_utils && !instance_debug_attach_status) {
        l.warning("Attaching SpyLog messenger failue");
    }

    if (has_debug_utils && instance_debug_attach_status) {
        l.debug("SpyLog messenger attached to instance");
    }
#endif

    VkInstance new_instance{};
    VkResult instance_create_status = vkCreateInstance(
        &vk_instance_info, impl.m_allocator->get_callbacks(), &new_instance
    );

    if (instance_create_status != VK_SUCCESS) {
        l.error(std::format(
            "Creating vulkan context failue: \"{}\"",
            vk_to_str(instance_create_status)
        ));
        return std::nullopt;
    }
    impl.m_vk_instance = new_instance;
    l.info(std::format(
        "Created instance with handle 0x{:x}",
        reinterpret_cast<size_t>(impl.m_vk_instance)
    ));
    l.set_name(std::format(
        "InstanceContext 0x{:x}", reinterpret_cast<size_t>(impl.m_vk_instance)
    ));

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    if (has_debug_utils) {
        debug_utils_load_status = pvk::DebugUtilsEXT::load(new_instance);
    }

    if (!debug_utils_load_status) {
        l.warning("Loading VK_EXT_debug_utils failue");
    }

    if (has_debug_utils && debug_utils_load_status) {
        enabled_extensions.emplace_back("VK_EXT_debug_utils");
        l.info("Extension \"VK_EXT_debug_utils\" is enabled");
    }

    bool primary_debugger_create_status = false;

    if (has_debug_utils && debug_utils_load_status &&
        impl.debugger != nullptr) {
        primary_debugger_create_status = impl.debugger->create_messenger(
            new_instance, impl.m_allocator->get_callbacks()
        );
    }

    if (primary_debugger_create_status) {
        l.debug("Primary debug messenger created");
    } else {
        l.warning("Primary debug messenger creation failue");
    }
#endif

    return InstanceContext(std::move(impl));
}

std::vector<PhysicalDevice> InstanceContext::get_devices() const
{
    std::vector<PhysicalDevice> output;

    auto devices = Impl::cast_from(this->impl).get_devices();
    output.reserve(devices.size());

    for (auto &device : devices) {
        output.emplace_back(from_native(device));
    }

    return output;
}

std::vector<VkPhysicalDevice> InstanceContext::Impl::get_devices() const
{
    uint32_t cnt_devices = 0;
    VkResult dev_enum_status =
        vkEnumeratePhysicalDevices(m_vk_instance, &cnt_devices, nullptr);
    if (dev_enum_status != VK_SUCCESS) {
        l.warning(std::format(
            "Counting vulkan physical devices failue: \"{}\"",
            vk_to_str(dev_enum_status)
        ));
        return {};
    }
    if (cnt_devices == 0) {
        return {};
    }

    std::vector<VkPhysicalDevice> devices;
    devices.resize(cnt_devices);
    dev_enum_status =
        vkEnumeratePhysicalDevices(m_vk_instance, &cnt_devices, devices.data());
    if (dev_enum_status != VK_SUCCESS) {
        l.warning(std::format(
            "Enumerating vulkan physical devices failue: "
            "\"{}\"",
            vk_to_str(dev_enum_status)
        ));
        return {};
    }
    return devices;
}

#if defined(PVK_USE_EXT_DEBUG_UTILS)
void InstanceContext::Impl::debug_utils_log_cb(
    void *user_data, Logger::Level level, const std::string &message
) noexcept
{
    InstanceContext::Impl &impl =
        *reinterpret_cast<InstanceContext::Impl *>(user_data);
    switch (level) {
    case Logger::Level::ERROR:
        impl.l.error(message);
        break;
    case Logger::Level::WARNING:
        impl.l.warning(message);
        break;
    case Logger::Level::INFO:
        [[fallthrough]];
    case Logger::Level::DEBUG:
        impl.l.debug(message);
        break;
    case Logger::Level::TRACE:
        impl.l.trace(message);
        break;
    }
    return;
}
#endif

InstanceContext::InstanceContext(InstanceContext &&other) noexcept
{
    auto &other_impl = InstanceContext::Impl::cast_from(other.impl);
    new (impl) InstanceContext::Impl(std::move(other_impl));
}

InstanceContext::Impl::Impl(InstanceContext::Impl &&other) noexcept
    : m_allocator(std::move(other.m_allocator)),
      m_vk_instance(std::move(other.m_vk_instance)), l(std::move(other.l))
#if (PVK_USE_EXT_DEBUG_UTILS)
      ,
      instance_spy(std::move(other.instance_spy)),
      debugger(std::move(other.debugger))
#endif
{
#if (PVK_USE_EXT_DEBUG_UTILS)
    instance_spy->get_logger().set_userdata(this);
    debugger->get_logger().set_userdata(this);
#endif
    other.m_vk_instance = VK_NULL_HANDLE;
}

} // namespace pvk
