#include <algorithm>
#include <array>
#include <format>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "pvk/vk_allocator.hh"
#include "pvk/vk_context.hh"
#include "pvk/vk_result.hh"

#if defined(PVK_USE_EXT_DEBUG_UTILS)
#include "pvk/extensions/debug_utils.hh"
#endif

#include "log.hh"
#include "string_pack.hh"

namespace pvk {

std::unordered_set<std::string> get_layer_extensions(const char *layer_name)
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
            log::warning(dup_warn);
            continue;
        }
        output.emplace(std::move(ext_name));
    }

    return output;
}

namespace {
static std::unordered_set<std::string> get_layers()
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
            log::warning(
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

static LayerExtMap
    get_layers_extensions(const std::unordered_set<std::string> &layer_names)
{
    LayerExtMap output;
    for (const auto &layer_name : layer_names) {
        std::unordered_set<std::string> layer_extensions =
            get_layer_extensions(layer_name.c_str());

        auto layer_extensions_list = output.find(layer_name);

        if (layer_extensions_list == std::end(output)) {
            std::unordered_set<std::string> new_extension_list;
            new_extension_list.reserve(layer_extensions.size());
            output.emplace(layer_name, std::move(new_extension_list));

            layer_extensions_list = output.find(layer_name);
            if (layer_extensions_list == std::end(output)) {
                log::error("No memory for layer's extension list");
                continue;
            }
        }

        for (auto &extension : layer_extensions) {
            layer_extensions_list->second.emplace(std::move(extension));
        }
    }
    return output;
}

static void dump_extensions_per_layer(const LayerExtMap &lay_exts)
{
    bool first = true;
    for (auto &layer : lay_exts) {
        if (first) {
            first = false;
        } else {
            log::info(std::format("| {: <49}|", ""));
        }

        log::info(std::format("| {: <49}|", layer.first));

        for (auto &extension : layer.second) {
            log::info(std::format("|   {: <47}|", std::string(extension)));
        }
        if (layer.second.size() == 0) {
            log::info(std::format("|   {: <47}|", "(No extensions)"));
        }
    }
}
}; // namespace

struct alignas(Context) Context::Impl
{
    static std::optional<Impl> create();

    Impl(Impl &&) = default;
    Impl &operator=(Impl &&) = default;

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    ~Impl()
    {
        if (m_allocator != nullptr) {
            vkDestroyInstance(m_vk_instance, m_allocator->get_callbacks());
        };
    }

    static Impl &cast_from(std::byte *data)
    {
        return *reinterpret_cast<Impl *>(data);
    }

  private:
    Impl() = default;

    VkInstance m_vk_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_vk_device = VK_NULL_HANDLE;

    std::unique_ptr<Allocator> m_allocator = nullptr;

#if(PVK_USE_EXT_DEBUG_UTILS)
    std::unique_ptr<DebugUtilsContext> debug = nullptr;
#endif

    std::stack<VkResult> m_vk_error_stack;
};

std::optional<Context::Impl> Context::Impl::create()
{
    std::unordered_set<std::string> full_extesions_list;
    static std::vector<std::string_view> enabled_layers;
    static std::vector<std::string_view> enabled_extensions;

    std::unordered_set<std::string> vk_layers = get_layers();

    auto implicit_extensions = get_layer_extensions(nullptr);
    for (auto &implicit_extension : implicit_extensions) {
        full_extesions_list.emplace(std::move(implicit_extension));
    }

    LayerExtMap layer_extensions = get_layers_extensions(vk_layers);

    for (auto &layer : layer_extensions) {
        for (auto &layer_ext_name : layer.second) {
            full_extesions_list.emplace(std::move(layer_ext_name));
        }
    }

    bool first = true;
    auto line_break_if_not_first = [&first]() {
        if (first) {
            first = false;
            return;
        }
        log::info("");
    };

    if (layer_extensions.size() != 0) {
        line_break_if_not_first();
        log::info(std::format("+{:=^50}+", "Layers"));
        dump_extensions_per_layer(layer_extensions);
        log::info(std::format("+{:=^50}+", ""));
    }

#if defined(PVK_USE_KHR_VALIDATION_LAYER)
    if (vk_layers.contains("VK_LAYER_KHRONOS_validation")) {
        line_break_if_not_first();
        log::info("Layer \"VK_LAYER_KHRONOS_validation\" is enabled");
        enabled_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    } else {
        log::warning("Layer \"VK_LAYER_KHRONOS_validation\" is not supported");
    }
#endif

    if (full_extesions_list.size() != 0) {
        line_break_if_not_first();
        log::info(std::format("+{:=^50}+", "All instance extensions"));
        for (auto &ext : full_extesions_list) {
            log::info(std::format("| {: <49}|", ext));
        }
        log::info(std::format("+{:=^50}+", ""));
    }

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    bool has_debug_utils = full_extesions_list.contains("VK_EXT_debug_utils");
    if (!has_debug_utils) {
        log::warning("VK_EXT_debug_utils extension is not supported: Ignore");
    }
    enabled_extensions.emplace_back("VK_EXT_debug_utils");
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

    auto new_allocator = std::make_unique<Allocator>();
    VkInstance new_instance{};

    VkResult instance_create_status = vkCreateInstance(
        &vk_instance_info, new_allocator->get_callbacks(), &new_instance
    );

    if (instance_create_status != VK_SUCCESS) {
        log::error(std::format(
            "Creating vulkan context failue: \"{}\"",
            vk_to_str(instance_create_status)
        ));
        return std::nullopt;
    }

#if defined(PVK_USE_EXT_DEBUG_UTILS)
    if (has_debug_utils) {
        if (!pvk::DebugUtilsEXT::load(new_instance)) {
            log::warning("Loading VK_EXT_debug_utils failue");
        } else {
            enabled_extensions.emplace_back("VK_EXT_debug_utils");
            line_break_if_not_first();
            log::info("Extension \"VK_EXT_debug_utils\" is enabled");
        }
    }
#endif

    uint32_t cnt_devices = 0;
    VkResult dev_enum_status =
        vkEnumeratePhysicalDevices(new_instance, &cnt_devices, nullptr);
    if (dev_enum_status != VK_SUCCESS) {
        log::error(std::format(
            "[ERROR]: Counting vulkan physical devices failue: \"{}\"",
            vk_to_str(dev_enum_status)
        ));
        return std::nullopt;
    }
    if (cnt_devices == 0) {
        log::error("[ERROR]: No single vulkan physical device found");
        return std::nullopt;
    }

    std::vector<VkPhysicalDevice> devices;
    devices.resize(cnt_devices);
    dev_enum_status =
        vkEnumeratePhysicalDevices(new_instance, &cnt_devices, devices.data());
    if (dev_enum_status != VK_SUCCESS) {
        log::error(std::format(
            "[ERROR]: Enumerating vulkan physical devices failue: "
            "\"{}\"",
            vk_to_str(dev_enum_status)
        ));
        return std::nullopt;
    }

    line_break_if_not_first();
    log::info(std::format("+{:=^50}+", "Devices"));
    for (auto device : devices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);
        log::info(std::format("|{: ^50}|", std::string(props.deviceName)));
    }
    log::info(std::format("+{:=^50}+", "="));

    Context::Impl impl;
    impl.m_vk_instance = new_instance;
    impl.m_vk_device = devices[0];
    VkPhysicalDeviceProperties dev_props{};
    vkGetPhysicalDeviceProperties(impl.m_vk_device, &dev_props);
    line_break_if_not_first();
    log::info(
        std::format("[INFO]: Selected device \"{}\"", dev_props.deviceName)
    );
    impl.m_allocator = std::move(new_allocator);

    return impl;
}

std::optional<Context> Context::create() noexcept
{
    Context output;
    auto impl = Context::Impl::create();
    if (!impl) {
        return std::nullopt;
    }

    new (output.impl) Context::Impl(std::move(*impl));
    return output;
}

Context::Context() = default;

Context::Context(Context &&other) noexcept
{
    new (impl) Context::Impl(std::move(Context::Impl::cast_from(other.impl)));
    std::ranges::fill(other.impl, std::byte(0));
}

Context &Context::operator=(Context &&other) noexcept
{
    std::swap(impl, other.impl);
    return *this;
}

Context::~Context()
{
    Context::Impl context(std::move(Impl::cast_from(impl)));
}
} // namespace pvk
