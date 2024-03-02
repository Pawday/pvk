#include <array>
#include <format>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cstdint>

#include "pvk/logger.hh"
#include "pvk/vk_allocator.hh"
#include "pvk/vk_instance_ctx.hh"
#include "pvk/vk_result.hh"

#include "pvk/log.hh"

#if defined(PVK_USE_EXT_DEBUG_UTILS)
#include "pvk/extensions/debug_utils.hh"
#endif

#include "string_pack.hh"
#include "vk_instance_ctx_impl.hh"

namespace pvk {

std::unordered_set<std::string>
    get_layer_extensions(Logger &l, const char *layer_name)
{
    uint32_t nb_extensions = 0;
    vkEnumerateInstanceExtensionProperties(layer_name, &nb_extensions, nullptr);
    std::vector<VkExtensionProperties> layer_exts(nb_extensions);
    vkEnumerateInstanceExtensionProperties(
        layer_name, &nb_extensions, layer_exts.data()
    );

    std::unordered_set<std::string> output;
    output.reserve(nb_extensions);
    for (auto &ext : layer_exts) {
        std::string ext_name(ext.extensionName);
        if (output.contains(ext_name)) {
            std::string dup_warn = std::format(
                "Extention \"{}\" mentioned more than once", ext_name
            );
            l.warning(dup_warn);
            continue;
        }
        output.emplace(std::move(ext_name));
    }

    return output;
}

namespace {
static std::unordered_set<std::string> get_layers(Logger &l)
{
    uint32_t nb_layers = 0;
    vkEnumerateInstanceLayerProperties(&nb_layers, nullptr);
    std::vector<VkLayerProperties> availableLayers(nb_layers);
    vkEnumerateInstanceLayerProperties(&nb_layers, availableLayers.data());

    std::unordered_set<std::string> output;
    output.reserve(nb_layers);

    for (auto &layer : availableLayers) {
        std::string layer_name(layer.layerName);
        if (output.contains(layer_name)) {
            l.warning(
                std::format("Layer {} mentioned more than once", layer_name)
            );
            continue;
        }
        output.emplace(layer_name);
    }
    return output;
}

using LayerExtMap =
    std::unordered_map<std::string, std::unordered_set<std::string>>;

static LayerExtMap get_layers_extensions(
    Logger &l, const std::unordered_set<std::string> &layer_names
)
{
    LayerExtMap output;
    for (const auto &layer_name : layer_names) {
        std::unordered_set<std::string> layer_extensions =
            get_layer_extensions(l, layer_name.c_str());

        auto layer_extensions_list = output.find(layer_name);

        if (layer_extensions_list == std::end(output)) {
            std::unordered_set<std::string> new_extension_list;
            new_extension_list.reserve(layer_extensions.size());
            output.emplace(layer_name, std::move(new_extension_list));

            layer_extensions_list = output.find(layer_name);
            if (layer_extensions_list == std::end(output)) {
                l.error("No memory for layer's extension list");
                continue;
            }
        }

        for (auto &extension : layer_extensions) {
            layer_extensions_list->second.emplace(std::move(extension));
        }
    }
    return output;
}

static void dump_extensions_per_layer(Logger &l, const LayerExtMap &lay_exts)
{
    bool first = true;
    for (auto &layer : lay_exts) {
        if (first) {
            first = false;
        } else {
            l.info(std::format("| {: <49}|", ""));
        }

        l.info(std::format("| {: <49}|", layer.first));

        for (auto &extension : layer.second) {
            l.info(std::format("|   {: <47}|", std::string(extension)));
        }
        if (layer.second.size() == 0) {
            l.info(std::format("|   {: <47}|", "(No extensions)"));
        }
    }
}

}; // namespace

std::optional<InstanceContext::Impl> InstanceContext::Impl::create()
{
    InstanceContext::Impl impl;
    impl.l.set_name("InstanceContext");

    std::unordered_set<std::string> full_extesions_list;
    static std::vector<std::string_view> enabled_layers;
    static std::vector<std::string_view> enabled_extensions;

    std::unordered_set<std::string> vk_layers = get_layers(impl.l);

    auto implicit_extensions = get_layer_extensions(impl.l, nullptr);
    for (auto &implicit_extension : implicit_extensions) {
        full_extesions_list.emplace(std::move(implicit_extension));
    }

    LayerExtMap layer_extensions = get_layers_extensions(impl.l, vk_layers);

    for (auto &layer : layer_extensions) {
        for (auto &layer_ext_name : layer.second) {
            full_extesions_list.emplace(std::move(layer_ext_name));
        }
    }

    bool first = true;
    auto line_break_if_not_first = [&first, &impl]() {
        if (first) {
            first = false;
            return;
        }
        impl.l.info("");
    };

    [[maybe_unused]] auto debug_line_break_if_not_first = [&first, &impl]() {
        if (first) {
            first = false;
            return;
        }
        impl.l.debug("");
    };

    if (layer_extensions.size() != 0) {
        line_break_if_not_first();
        impl.l.info(std::format("+{:=^50}+", " Layers "));
        dump_extensions_per_layer(impl.l, layer_extensions);
        impl.l.info(std::format("+{:=^50}+", ""));
    }

#if defined(PVK_USE_KHR_VALIDATION_LAYER)
    if (vk_layers.contains("VK_LAYER_KHRONOS_validation")) {
        line_break_if_not_first();
        impl.l.info("Layer \"VK_LAYER_KHRONOS_validation\" is enabled");
        enabled_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    } else {
        impl.l.warning("Layer \"VK_LAYER_KHRONOS_validation\" is not supported"
        );
    }
#endif

    if (full_extesions_list.size() != 0) {
        line_break_if_not_first();
        impl.l.info(std::format("+{:=^50}+", " All instance extensions "));
        for (auto &ext : full_extesions_list) {
            impl.l.info(std::format("| {: <49}|", ext));
        }
        impl.l.info(std::format("+{:=^50}+", ""));
    }

    impl.m_allocator = std::make_unique<Allocator>();

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    bool has_debug_utils = full_extesions_list.contains("VK_EXT_debug_utils");
    bool debug_utils_load_status = false;
    if (has_debug_utils) {
        enabled_extensions.emplace_back("VK_EXT_debug_utils");
    } else {
        impl.l.warning("VK_EXT_debug_utils extension is not supported: Ignore");
    }

    if (has_debug_utils) {
        impl.instance_spy =
            DebugUtilsContext::create(impl.m_allocator->get_callbacks());
        impl.instance_spy->get_logger().set_name("VkInstance Spy");
        impl.debugger =
            DebugUtilsContext::create(impl.m_allocator->get_callbacks());
        impl.debugger->get_logger().set_name("Debug Utils");
    }

    if (has_debug_utils && impl.instance_spy == nullptr) {
        impl.l.warning("Creating early debug messenger failue");
    }

    if (has_debug_utils && impl.debugger == nullptr) {
        impl.l.warning("Creating primary debug messenger failue");
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
        impl.l.warning("Attaching early messenger failue");
    }

    if (has_debug_utils && instance_debug_attach_status) {
        debug_line_break_if_not_first();
        impl.l.debug("Early messenger attached to instance");
    }
#endif

    VkInstance new_instance{};
    VkResult instance_create_status = vkCreateInstance(
        &vk_instance_info, impl.m_allocator->get_callbacks(), &new_instance
    );

    if (instance_create_status != VK_SUCCESS) {
        impl.l.error(std::format(
            "Creating vulkan context failue: \"{}\"",
            vk_to_str(instance_create_status)
        ));
        return std::nullopt;
    }
    impl.m_vk_instance = new_instance;

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    if (has_debug_utils) {
        debug_utils_load_status = pvk::DebugUtilsEXT::load(new_instance);
    }

    if (!debug_utils_load_status) {
        impl.l.warning("Loading VK_EXT_debug_utils failue");
    }

    if (has_debug_utils && debug_utils_load_status) {
        enabled_extensions.emplace_back("VK_EXT_debug_utils");
        line_break_if_not_first();
        impl.l.info("Extension \"VK_EXT_debug_utils\" is loaded");
    }

    bool primary_debugger_create_status = false;

    if (has_debug_utils && debug_utils_load_status &&
        impl.debugger != nullptr) {
        primary_debugger_create_status = impl.debugger->create_messenger(
            new_instance, impl.m_allocator->get_callbacks()
        );
    }

    if (primary_debugger_create_status) {
        impl.l.debug("Primary debug messenger created");
    } else {
        impl.l.warning("Primary debug messenger creation failue");
    }
#endif

    return impl;
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

} // namespace pvk
