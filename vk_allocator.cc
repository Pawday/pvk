#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <iterator>

#if defined(USE_WINDOWS_ALIGNED_ALLOC)
#include <corecrt_malloc.h>
#endif

#include "pvk/vk_allocator.hh"

namespace {
static void *aligned_alloc_wrap(size_t alignment, size_t aligned_size)
{
#if defined(USE_WINDOWS_ALIGNED_ALLOC)
    return _aligned_malloc(aligned_size, alignment);
#else
    return std::aligned_alloc(alignment, aligned_size);
#endif
}

static void aligned_free_wrap(void *p)
{
#if defined(USE_WINDOWS_ALIGNED_ALLOC)
    _aligned_free(p);
#else
    free(p);
#endif
}

static void
warn_memsize_align(size_t size, size_t alignment, size_t aligned_size)
{
    return;
    std::cerr << std::format(
        "[TRACE]: VkAllocator: Driver requested undivisibly {} "
        "bytes by alignment {}, allocating {} bytes instead\n",
        size,
        alignment,
        aligned_size
    );
}
} // namespace

struct VkAllocator::ImplFriend
{
    static void *vkAllocationFunction(
        void *allocator_p,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope
    )
    {
        VkAllocator *allocator = reinterpret_cast<VkAllocator *>(allocator_p);

        size_t aligned_size = size;

        if (size % alignment != 0) {
            aligned_size = ((size / alignment) + 1) * alignment;
            warn_memsize_align(size, alignment, aligned_size);
        }

        /*
         * Nvidia Driver rely on allocation padding with 32 times more memory
         * than requested.
         * Discusting but okay, we can give them more
         *
         * also VK_LAYER_NV_optimus crushes when
         * nevidia_memory_workaround_multiplyer < 64
         */
#if defined(GIVE_NEVIDIA_MORE_MEMORY)
        constexpr size_t nevidia_memory_workaround_multiplyer = 64;
        aligned_size += alignment * nevidia_memory_workaround_multiplyer;
#endif

        void *new_block = aligned_alloc_wrap(alignment, aligned_size);
        if (new_block == nullptr) {
            return nullptr;
        }

        VkAllocator::MemBlock new_block_meta;
        new_block_meta.align = alignment;
        new_block_meta.size = aligned_size;
        new_block_meta.vk_scope = allocationScope;
        size_t new_block_addr = reinterpret_cast<size_t>(new_block);
        allocator->m_blocks.emplace(new_block_addr, new_block_meta);
        return new_block;
    }

    static void vkFreeFunction(void *allocator_p, void *pMemory)
    {
        if (pMemory == nullptr) {
            return;
        }

        VkAllocator *allocator = reinterpret_cast<VkAllocator *>(allocator_p);

        size_t addr_to_free = reinterpret_cast<size_t>(pMemory);
        auto orig_block_it = allocator->m_blocks.find(addr_to_free);
        if (orig_block_it == std::end(allocator->m_blocks)) {
            std::cerr << std::format(
                "VkAllocator: freeing of nonallocated addres 0x{:x}\n",
                addr_to_free
            );
            return;
        }

        allocator->m_blocks.erase(orig_block_it);
        aligned_free_wrap(pMemory);
    }

    static void *vkReallocationFunction(
        void *allocator_p,
        void *original_p,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope
    )
    {
        VkAllocator *allocator = reinterpret_cast<VkAllocator *>(allocator_p);
        if (original_p == nullptr) {
            return vkAllocationFunction(
                allocator_p, size, alignment, allocationScope
            );
        }

        size_t original_addr = reinterpret_cast<size_t>(original_p);
        auto orig_block_it = allocator->m_blocks.find(original_addr);
        if (orig_block_it == std::end(allocator->m_blocks)) {
            std::cerr << std::format(
                "VkAllocator: reallocating of nonallocated addres 0x{:x}\n",
                original_addr
            );
            return nullptr;
        }

        auto new_block =
            vkAllocationFunction(allocator_p, size, alignment, allocationScope);
        if (new_block == nullptr) {
            return nullptr;
        }

        std::memcpy(new_block, original_p, size);
        vkFreeFunction(allocator_p, original_p);
        return new_block;
    }

    static void vkInternalAllocationNotification(
        void *allocator_p,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope
    )
    {
        VkAllocator *allocator = reinterpret_cast<VkAllocator *>(allocator_p);
        (void)allocator;
        (void)size;
        (void)allocationType;
        (void)allocationScope;
        return;
    }

    static void vkInternalFreeNotification(
        void *allocator_p,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope
    )
    {
        VkAllocator *allocator = reinterpret_cast<VkAllocator *>(allocator_p);
        (void)allocator;
        (void)size;
        (void)allocationType;
        (void)allocationScope;
        return;
    }
};

VkAllocator::VkAllocator() noexcept
{
    m_callbacks.pUserData = this;
    m_callbacks.pfnAllocation = VkAllocator::ImplFriend::vkAllocationFunction;
    m_callbacks.pfnReallocation =
        VkAllocator::ImplFriend::vkReallocationFunction;
    m_callbacks.pfnFree = VkAllocator::ImplFriend::vkFreeFunction;

    m_callbacks.pfnInternalAllocation =
        VkAllocator::ImplFriend::vkInternalAllocationNotification;
    m_callbacks.pfnInternalFree =
        VkAllocator::ImplFriend::vkInternalFreeNotification;
}