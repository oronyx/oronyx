#pragma once

#ifndef __ORNYX__MEM__IMPL__
#define __ORNYX__MEM__IMPL__

#include <oronyx/arch/mem.hpp>

namespace onx
{
    static constexpr size_t PAGE_SIZE = 4096;
    static constexpr size_t PAGE_SHIFT = 12;
    static constexpr size_t PAGE_MASK = PAGE_SIZE - 1;

    /* x86 paging flags */
    constexpr uint64_t PRESENT = 1ULL << 0;
    constexpr uint64_t WRITABLE = 1ULL << 1;
    constexpr uint64_t USER = 1ULL << 2;
    static constexpr uint64_t WRITE_THROUGH = 1ULL << 3;
    static constexpr uint64_t CACHE_DISABLE = 1ULL << 4;
    static constexpr uint64_t ACCESSED = 1ULL << 5;
    static constexpr uint64_t DIRTY = 1ULL << 6;
    static constexpr uint64_t HUGE = 1ULL << 7;
    static constexpr uint64_t GLOBAL = 1ULL << 8;
    static constexpr uint64_t NX = 1ULL << 63;

    template<>
    struct mm_trait<x86_64>
    {
        static uint64_t *alloc_page() noexcept;

        static void flush_tlb(void *addr) noexcept;

        static void init(volatile limine_hhdm_request *hhdm,
                         volatile limine_memmap_request *mmap) noexcept;

        static void map_page(uintptr_t virt, uintptr_t phys, uint64_t flags) noexcept;
    };
}

#endif
