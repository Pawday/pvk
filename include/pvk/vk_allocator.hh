#pragma once

#include <cstddef>

#include <unordered_map>

#include "vk_api.hh"

struct VkAllocator
{
    struct MemBlock
    {
        using Address = size_t;
        size_t size;
        size_t align;
        VkSystemAllocationScope vk_scope;
    };

    VkAllocator() noexcept;
    VkAllocator(const VkAllocator &) = delete;
    VkAllocator &operator=(const VkAllocator &) = delete;
    VkAllocator(VkAllocator &&) = delete;
    VkAllocator &operator=(VkAllocator &&) = delete;

    const VkAllocationCallbacks *get_callbacks() const
    {
        return &m_callbacks;
    }

    std::unordered_map<MemBlock::Address, MemBlock> get_allocated_blocks()
    {
        return m_blocks;
    }

  private:
    struct ImplFriend;
    std::unordered_map<MemBlock::Address, MemBlock> m_blocks;
    VkAllocationCallbacks m_callbacks{};
};
