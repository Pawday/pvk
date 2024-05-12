#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iterator>

#include "pvk/log.hh"

#include "pvk/internal/vk_allocator.hh"

namespace {
static void *aligned_alloc_wrap(size_t alignment, size_t aligned_size)
{
#if defined(PVK_USE_WINDOWS_ALIGNED_ALLOC)
    return _aligned_malloc(aligned_size, alignment);
#else
    return std::aligned_alloc(alignment, aligned_size);
#endif
}

static void aligned_free_wrap(void *p)
{
#if defined(PVK_USE_WINDOWS_ALIGNED_ALLOC)
    _aligned_free(p);
#else
    free(p);
#endif
}

#if defined(PVK_ALLOCATOR_ENABLE_ALIGN_MISMATCH_DEBUG)
static void
    warn_memsize_align(size_t size, size_t alignment, size_t aligned_size)
{
    pvk::warning(
        "Driver requested undivisibly {} "
        "bytes by alignment {}, allocating {} bytes instead",
        size,
        alignment,
        aligned_size);
}
#endif

} // namespace

namespace pvk {

struct Allocator::ImplFriend
{
    static void *VKAPI_CALL vkAllocationFunction(
        void *allocator_p,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope)
    {
        Allocator *allocator = reinterpret_cast<Allocator *>(allocator_p);

        size_t aligned_size = size;

        if (size % alignment != 0) {
            aligned_size = ((size / alignment) + 1) * alignment;
#if defined(PVK_ALLOCATOR_ENABLE_ALIGN_MISMATCH_DEBUG)
            warn_memsize_align(size, alignment, aligned_size);
#endif
        }

        void *new_block = aligned_alloc_wrap(alignment, aligned_size);
        if (new_block == nullptr) {
            return nullptr;
        }

        Allocator::MemBlock new_block_meta;
        new_block_meta.align = alignment;
        new_block_meta.size = aligned_size;
        new_block_meta.vk_scope = allocationScope;
        size_t new_block_addr = reinterpret_cast<size_t>(new_block);
        allocator->m_blocks.emplace(new_block_addr, new_block_meta);
        return new_block;
    }

    static void VKAPI_CALL vkFreeFunction(void *allocator_p, void *pMemory)
    {
        if (pMemory == nullptr) {
            return;
        }

        Allocator *allocator = reinterpret_cast<Allocator *>(allocator_p);

        size_t addr_to_free = reinterpret_cast<size_t>(pMemory);
        auto orig_block_it = allocator->m_blocks.find(addr_to_free);
        if (orig_block_it == std::end(allocator->m_blocks)) {
            pvk::warning(
                "VkAllocator: request freeing of nonallocated addres 0x{:x}",
                addr_to_free);
            return;
        }

        allocator->m_blocks.erase(orig_block_it);
        aligned_free_wrap(pMemory);
    }

    static void *VKAPI_CALL vkReallocationFunction(
        void *allocator_p,
        void *original_p,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope)
    {
        Allocator *allocator = reinterpret_cast<Allocator *>(allocator_p);
        if (original_p == nullptr) {
            return vkAllocationFunction(
                allocator_p, size, alignment, allocationScope);
        }

        size_t original_addr = reinterpret_cast<size_t>(original_p);
        auto orig_block_it = allocator->m_blocks.find(original_addr);
        if (orig_block_it == std::end(allocator->m_blocks)) {
            pvk::warning(
                "VkAllocator: reallocating of nonallocated addres 0x{:x}\n",
                original_addr);
            return nullptr;
        }

        auto new_block =
            vkAllocationFunction(allocator_p, size, alignment, allocationScope);
        if (new_block == nullptr) {
            return nullptr;
        }

        std::memcpy(new_block, original_p, orig_block_it->second.size);
        vkFreeFunction(allocator_p, original_p);
        return new_block;
    }

    static void VKAPI_CALL vkInternalAllocationNotification(
        void *allocator_p,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope)
    {
        Allocator *allocator = reinterpret_cast<Allocator *>(allocator_p);
        (void)allocator;
        (void)size;
        (void)allocationType;
        (void)allocationScope;
        return;
    }

    static void VKAPI_CALL vkInternalFreeNotification(
        void *allocator_p,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope)
    {
        Allocator *allocator = reinterpret_cast<Allocator *>(allocator_p);
        (void)allocator;
        (void)size;
        (void)allocationType;
        (void)allocationScope;
        return;
    }
};

Allocator::Allocator() noexcept
{
    m_callbacks.pUserData = this;
    m_callbacks.pfnAllocation = Allocator::ImplFriend::vkAllocationFunction;
    m_callbacks.pfnReallocation = Allocator::ImplFriend::vkReallocationFunction;
    m_callbacks.pfnFree = Allocator::ImplFriend::vkFreeFunction;

    m_callbacks.pfnInternalAllocation =
        Allocator::ImplFriend::vkInternalAllocationNotification;
    m_callbacks.pfnInternalFree =
        Allocator::ImplFriend::vkInternalFreeNotification;
}
} // namespace pvk
