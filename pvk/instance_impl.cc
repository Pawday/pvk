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

#include <pvk/device_context.hh>
#include <pvk/instance.hh>
#include <pvk/log.hh>
#include <pvk/logger.hh>
#include <pvk/vk_allocator.hh>
#include <pvk/vk_api.hh>

#if defined(PVK_USE_EXT_DEBUG_UTILS)
#include <pvk/extensions/debug_utils.hh>
#include <pvk/extensions/debug_utils_context.hh>
#endif

#include "pvk/device_context_impl.hh"
#include "pvk/instance_impl.hh"
#include "pvk/layer_utils.hh"
#include "pvk/log_utils.hh"
#include "pvk/result.hh"
#include "pvk/string_pack.hh"

namespace pvk {

Instance::Instance(Instance::Impl &&impl_obj) noexcept
{
    new (this->impl) Instance::Impl(std::move(impl_obj));
}

Instance &Instance::operator=(Instance &&other) noexcept
{
    std::swap(this->impl, other.impl);
    return *this;
}

Instance::~Instance() noexcept
{
    Impl::cast_from(*this).~Impl();
}

Instance::Impl::~Impl() noexcept
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

std::optional<Instance> Instance::create() noexcept
{
    return Instance::Impl::create();
}

std::optional<Instance> Instance::Impl::create()
{
    Instance::Impl impl;
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
        l.notice("Layer \"VK_LAYER_KHRONOS_validation\" is not supported");
    }
#endif
    if (full_extesions_list.size() != 0) {

        std::string label = "All instance extensions";
        size_t max_line_size = label.size() + 2;
        std::vector<std::string> lines;
        lines.reserve(full_extesions_list.size());

        for (auto &ext : full_extesions_list) {
            lines.emplace_back(ext);
        }
        auto max_line = std::ranges::max_element(
            lines, [](const auto &lhs, const auto &rhs) {
                return lhs.size() < rhs.size();
            });
        if (max_line != std::end(lines) && max_line_size < max_line->size()) {
            max_line_size = max_line->size();
        }

        std::ranges::sort(lines);

        l.info(box_title(label, max_line_size));
        for (auto &line : lines) {
            l.info(box_entry(line, max_line_size));
        }
        l.info(box_foot(max_line_size));
    }

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    bool has_debug_utils = full_extesions_list.contains("VK_EXT_debug_utils");
    bool debug_utils_load_status = false;
    if (has_debug_utils) {
        enabled_extensions.emplace_back("VK_EXT_debug_utils");
    } else {
        l.notice("VK_EXT_debug_utils extension is not supported: Ignore");
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
        &vk_instance_info, impl.m_allocator->get_callbacks(), &new_instance);

    if (instance_create_status != VK_SUCCESS) {
        l.warning(std::format(
            "Creating vulkan context failue: \"{}\"",
            vk_to_str(instance_create_status)));
        return std::nullopt;
    }
    impl.m_vk_instance = new_instance;
    l.info(std::format(
        "Created instance with handle 0x{:x}",
        reinterpret_cast<size_t>(impl.m_vk_instance)));
    l.set_name(std::format(
        "InstanceContext 0x{:x}",
        reinterpret_cast<size_t>(impl.m_vk_instance)));

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
            new_instance, impl.m_allocator->get_callbacks());
    }

    if (primary_debugger_create_status) {
        l.debug("Primary debug messenger created");
    } else {
        l.warning("Primary debug messenger creation failue");
    }
#endif

    if (!impl.load_devices()) {
        l.warning("Loading physical device list failue");
        return std::nullopt;
    }

    return Instance(std::move(impl));
}

bool Instance::Impl::load_devices()
{
    uint32_t cnt_devices = 0;
    VkResult dev_enum_status =
        vkEnumeratePhysicalDevices(m_vk_instance, &cnt_devices, nullptr);
    if (dev_enum_status != VK_SUCCESS) {
        l.warning(std::format(
            "Counting vulkan physical devices failue: \"{}\"",
            vk_to_str(dev_enum_status)));
        return false;
    }
    if (cnt_devices == 0) {
        return false;
    }

    std::vector<VkPhysicalDevice> devices;
    devices.resize(cnt_devices);
    dev_enum_status =
        vkEnumeratePhysicalDevices(m_vk_instance, &cnt_devices, devices.data());
    if (dev_enum_status != VK_SUCCESS) {
        l.warning(std::format(
            "Enumerating vulkan physical devices failue: "
            "\"{}\"",
            vk_to_str(dev_enum_status)));
        return {};
    }

    m_devices = devices;
    return true;
}

#if defined(PVK_USE_EXT_DEBUG_UTILS)
void Instance::Impl::debug_utils_log_cb(
    void *user_data,
    Logger::Level level,
    const std::string_view &message) noexcept
{
    Instance::Impl &impl =
        *reinterpret_cast<Instance::Impl *>(user_data);

    switch (level) {
    case Logger::Level::FATAL:
        impl.l.fatal(message);
        break;
    case Logger::Level::ERROR:
        impl.l.error(message);
        break;
    case Logger::Level::WARNING:
        impl.l.warning(message);
        break;
    case Logger::Level::NOTICE:
        impl.l.notice(message);
        break;
    case Logger::Level::INFO:
        impl.l.info(message);
        break;
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

Instance::Instance(Instance &&other) noexcept
{
    auto &other_impl = Instance::Impl::cast_from(other);
    new (impl) Instance::Impl(std::move(other_impl));
}

Instance::Impl::Impl(Instance::Impl &&other) noexcept
    : m_allocator(std::move(other.m_allocator)),
      m_vk_instance(std::move(other.m_vk_instance)), l(std::move(other.l)),
      m_devices(std::move(other.m_devices))
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

size_t Instance::get_device_count() const noexcept
{
    return Impl::cast_from(*this).get_device_count();
}
size_t Instance::Impl::get_device_count() const noexcept
{
    return this->m_devices.size();
}

std::optional<DeviceContext>
    Instance::get_device(size_t device_idx) const noexcept
{
    return Impl::cast_from(*this).get_device(device_idx);
}

std::optional<DeviceContext>
    Instance::Impl::get_device(size_t device_idx) const noexcept
{
    if (device_idx >= m_devices.size()) {
        return std::nullopt;
    }

    VkPhysicalDevice physical_device = m_devices[device_idx];

    DeviceContext::Impl i(std::move(physical_device));

    return DeviceContext(std::move(i));
}

} // namespace pvk
