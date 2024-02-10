#pragma once

#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "vk_api.hh"

#include "vk_allocator.hh"

struct VKContext
{
    static std::optional<VKContext> create() noexcept;
    VKContext(VKContext &&) = default;
    VKContext &operator=(VKContext &&) = default;
    ~VKContext();

    VKContext(const VKContext &) = delete;
    VKContext &operator=(const VKContext &) = delete;

    std::stack<VkResult> get_error_stack() const
    {
        return m_vk_error_stack;
    }

  private:
    VKContext() = default;

    struct Detail;

    std::unique_ptr<VkAllocator> m_allocator = nullptr;
    std::vector<VkExtensionProperties> m_vk_extensions;
    std::vector<VkLayerProperties> m_vk_layers;
    std::unordered_map<std::string, std::vector<VkExtensionProperties>>
        m_layer_extensions;

    VkInstance m_vk_instance{};
    std::stack<VkResult> m_vk_error_stack;
};
