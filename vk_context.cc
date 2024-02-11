#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "pvk/vk_allocator.hh"
#include "pvk/vk_context.hh"
#include "pvk/vk_result.hh"

struct VKContext::Detail
{
    static std::vector<VkLayerProperties> get_instance_layers()
    {
        uint32_t nb_layers = 0;
        vkEnumerateInstanceLayerProperties(&nb_layers, nullptr);
        std::vector<VkLayerProperties> availableLayers(nb_layers);
        vkEnumerateInstanceLayerProperties(&nb_layers, availableLayers.data());
        return availableLayers;
    }

    static std::vector<VkExtensionProperties>
        get_instance_extensions(const char *layer_name)
    {
        uint32_t nb_extensions = 0;
        vkEnumerateInstanceExtensionProperties(
            layer_name, &nb_extensions, nullptr
        );
        std::vector<VkExtensionProperties> availableLayers(nb_extensions);
        vkEnumerateInstanceExtensionProperties(
            layer_name, &nb_extensions, availableLayers.data()
        );
        return availableLayers;
    }

    static std::unordered_map<std::string, std::vector<VkExtensionProperties>>
        get_instance_extensions(const std::vector<VkLayerProperties> &layers)
    {
        std::unordered_map<std::string, std::vector<VkExtensionProperties>>
            output;
        for (const auto &layer : layers) {
            std::string layer_name = layer.layerName;
            std::vector<VkExtensionProperties> layer_extensions =
                get_instance_extensions(layer_name.c_str());
            output.emplace(layer_name, std::move(layer_extensions));
        }
        return output;
    }

    static void
        dump_extension(const std::vector<VkExtensionProperties> &extensions)
    {
        for (auto &gext : extensions) {
            std::cout << std::format(
                "| {: <49}|\n", std::string(gext.extensionName)
            );
        }
    }

    using ExtMap =
        std::unordered_map<std::string, std::vector<VkExtensionProperties>>;
    static void dump_extensions_per_layer(const ExtMap &lay_exts)
    {
        for (auto &layer : lay_exts) {
            std::cout << std::format("| {: <49}|\n", layer.first);
            for (auto &extension : layer.second) {
                std::cout << std::format(
                    "|   {: <47}|\n", std::string(extension.extensionName)
                );
            }
            if (layer.second.size() == 0) {
                std::cout << std::format("|   {: <47}|\n", "(No extensions)");
            }
            std::cout << std::format("| {: <49}|\n", "");
        }
    }
};

struct StringList
{
    StringList(const StringList &) = delete;
    StringList &operator=(const StringList &) = delete;

    StringList(StringList &) = default;
    StringList &operator=(StringList &&) = default;

    template <size_t EXTEND = std::dynamic_extent>
    static constexpr std::optional<StringList>
        create(const std::span<std::string_view, EXTEND> &strings) noexcept
    try {
        const size_t total_size = [&strings]() {
            size_t output = 0;
            for (auto &s : strings) {
                output += s.size() + 1;
            }
            return output;
        }();

        std::vector<size_t> offsets;
        offsets.reserve(strings.size());
        std::vector<char> data;
        data.reserve(total_size);

        size_t offset = 0;

        for (auto &s : strings) {
            offsets.emplace_back(offset);
            std::copy(begin(s), end(s), std::back_inserter(data));
            data.emplace_back(0);
            offset += s.size() + 1;
        }

        // Just in case
        data.emplace_back(0);

        StringList output;
        output.data = std::move(data);
        output.offsets = std::move(offsets);
        return output;

    } catch (...) {
        return std::nullopt;
    }

    ~StringList() = default;

    std::vector<const char *> get() &
    {
        std::vector<const char *> output;
        output.reserve(offsets.size());
        auto make_pointer = [this](size_t offset) -> const char * {
            return data.data() + offset;
        };
        auto pointers = offsets | std::views::transform(make_pointer);
        std::ranges::copy(pointers, std::back_inserter(output));
        return output;
    }

  private:
    StringList() noexcept = default;
    std::vector<char> data;
    std::vector<size_t> offsets;
};

std::optional<VKContext> VKContext::create() noexcept
{
    static std::vector<std::string_view> required_layers;
    // required_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    // required_layers.emplace_back("VK_LAYER_NV_optimus");

    static std::vector<std::string_view> required_extensions;
    required_extensions.emplace_back("VK_EXT_debug_utils");

    std::vector<VkLayerProperties> vk_layers = Detail::get_instance_layers();
    std::vector<VkExtensionProperties> instance_extensions =
        Detail::get_instance_extensions(nullptr);

    std::cout << std::format("+{:-^50}+\n", "Instance Extensions");
    Detail::dump_extension(instance_extensions);
    std::cout << std::format("+{:-^50}+\n", "-");
    Detail::ExtMap layer_extensions =
        Detail::get_instance_extensions(vk_layers);
    if (layer_extensions.size() != 0) {
        std::cout << '\n';
        std::cout << std::format("+{:=^50}+\n", "Layers");
        Detail::dump_extensions_per_layer(layer_extensions);
        std::cout << std::format("+{:=^50}+\n", "");
    }

    auto en_layer_names = StringList::create(std::span(required_layers));
    auto en_ext_names = StringList::create(std::span(required_extensions));
    if (!en_layer_names || !en_ext_names) {
        return std::nullopt;
    }

    auto en_layer_names_ptrs = en_layer_names->get();
    auto en_ext_names_ptrs = en_ext_names->get();

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

    auto new_allocator = std::make_unique<VkAllocator>();
    VkInstance new_instance{};
    VkResult instance_create_status = vkCreateInstance(
        &vk_instance_info, new_allocator->get_callbacks(), &new_instance
    );

    if (instance_create_status != VK_SUCCESS) {
        std::cerr << std::format(
            "Creating vulkan context failue: \"{}\"\n",
            vk_to_str(instance_create_status)
        );
        return std::nullopt;
    }

    std::cout << '\n';
    std::cout << std::format("+{:=^50}+\n", "Devices");

    uint32_t cnt_devices = 0;
    VkResult dev_enum_status =
        vkEnumeratePhysicalDevices(new_instance, &cnt_devices, nullptr);
    if (dev_enum_status != VK_SUCCESS) {
        std::cerr << std::format(
            "[ERROR]: Counting vulkan physical devices failue: \"{}\"\n",
            vk_to_str(dev_enum_status)
        );
        return std::nullopt;
    }
    if (cnt_devices == 0) {
        std::cerr << "[ERROR]: No single vulkan physical device found\n";
        return std::nullopt;
    }

    std::vector<VkPhysicalDevice> devices;
    devices.resize(cnt_devices);
    dev_enum_status =
        vkEnumeratePhysicalDevices(new_instance, &cnt_devices, devices.data());
    if (dev_enum_status != VK_SUCCESS) {
        std::cerr << std::format(
            "[ERROR]: Enumerating vulkan physical devices failue: "
            "\"{}\"\n",
            vk_to_str(dev_enum_status)
        );
        return std::nullopt;
    }

    for (auto device : devices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << std::format("|{: ^50}|\n", std::string(props.deviceName));
    }
    std::cout << std::format("+{:=^50}+\n", "=");

    VKContext output;
    output.m_vk_instance = new_instance;

    output.m_vk_device = devices[0];
    VkPhysicalDeviceProperties dev_props{};
    vkGetPhysicalDeviceProperties(output.m_vk_device, &dev_props);
    std::cout << std::format(
        "[INFO]: Selected device \"{}\"\n", dev_props.deviceName
    );
    output.m_allocator = std::move(new_allocator);
    return output;
}

VKContext::~VKContext()
{
    if (m_allocator != nullptr) {
        vkDestroyInstance(m_vk_instance, m_allocator->get_callbacks());
    };
}
