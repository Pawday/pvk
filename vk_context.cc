#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cstdint>

#include "vk_context.hh"
#include "vk_result.hh"

struct VKContext::Detail
{
    static std::vector<VkLayerProperties> get_layers()
    {
        uint32_t nb_layers = 0;
        vkEnumerateInstanceLayerProperties(&nb_layers, nullptr);
        std::vector<VkLayerProperties> availableLayers(nb_layers);
        vkEnumerateInstanceLayerProperties(&nb_layers, availableLayers.data());
        return availableLayers;
    }

    static std::vector<VkExtensionProperties>
    get_extensions(const char *layer_name)
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
    get_extensions_for_layers(const std::vector<VkLayerProperties> &layers)
    {
        std::unordered_map<std::string, std::vector<VkExtensionProperties>>
            output;
        for (const auto &layer : layers) {
            std::string layer_name = layer.layerName;
            std::vector<VkExtensionProperties> layer_extensions =
                get_extensions(layer_name.c_str());
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
            std::cout << std::format("+{:-^50}+\n", layer.first);
            for (auto &extension : layer.second) {
                std::cout << std::format(
                    "| {: <49}|\n", std::string(extension.extensionName)
                );
            }
            if (layer.second.size() == 0) {
                std::cout << std::format("| {: ^49}|\n", "(No extensions)");
            }
            std::cout << std::format("+{:-^50}+\n", "-");
        }
    }
};

static constexpr bool use_khr_validation_layers = true;

static std::vector<std::string> required_layers{"VK_LAYER_KHRONOS_validation"};
static std::vector<std::string> required_extensions{"VK_EXT_debug_utils"};

std::optional<VKContext> VKContext::create() noexcept
{
    std::vector<VkLayerProperties> vk_layers = Detail::get_layers();
    std::vector<VkExtensionProperties> vk_extensions =
        Detail::get_extensions(nullptr);

    std::cout << std::format("+{:-^50}+\n", "Global Extensions");
    Detail::dump_extension(vk_extensions);
    std::cout << std::format("+{:-^50}+\n", "-");

    std::unordered_map<std::string, std::vector<VkExtensionProperties>>
        layer_extensions = Detail::get_extensions_for_layers(vk_layers);
    std::cout << "Layer extensions:\n";
    Detail::dump_extensions_per_layer(layer_extensions);

    VkApplicationInfo vk_app_info{};
    VkInstanceCreateInfo vk_instance_info{};
    vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_app_info.pApplicationName = "Some name";
    vk_app_info.apiVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    vk_instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_instance_info.pApplicationInfo = &vk_app_info;

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

    VKContext output;
    output.m_vk_instance = new_instance;
    output.m_allocator = std::move(new_allocator);
    return output;
}

VKContext::~VKContext()
{
    if (m_allocator != nullptr) {
        vkDestroyInstance(m_vk_instance, m_allocator->get_callbacks());
    };
}
