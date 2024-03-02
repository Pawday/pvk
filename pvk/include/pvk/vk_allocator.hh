#pragma once

#include <cstddef>

#include <unordered_map>

#include "vk_api.hh"

namespace pvk {

struct Allocator
{
    struct MemBlock
    {
        using Address = size_t;
        size_t size;
        size_t align;
        VkSystemAllocationScope vk_scope;
    };

    Allocator() noexcept;

    // new Allocator passes its addres at creation
    // it should be alive at that specfic addres until it death

    Allocator(const Allocator &) = delete;
    Allocator &operator=(const Allocator &) = delete;
    Allocator(Allocator &&) = delete;
    Allocator &operator=(Allocator &&) = delete;

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

} // namespace pvk
