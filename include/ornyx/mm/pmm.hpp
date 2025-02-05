#pragma once

#ifndef __ORNYX__MM__PMM__
#define __ORNYX__MM__PMM__

#include <ornyx/boot/limine.h>
#include <types.hpp>

namespace onx::mm
{
    struct Region
    {
        uintptr_t base;
        size_t len;
        bool is_free; /* maybe this'll be a uint8_t flags instead */
    };

    static constexpr size_t PAGE_SIZE = 4096;
    static constexpr size_t MAX_REGIONS = 64;

    /* TODO: make this dynamic, not bump */
    struct PMM
    {
        static void init(const volatile limine_memmap_request* mmap, uint64_t offset) noexcept;

        static uintptr_t alloc_pages(size_t count = 1) noexcept;

        static void free_pages(uintptr_t base, size_t count = 1) noexcept;

        /* debug helpers */
        static void print_regions() noexcept;
        static size_t get_free_memory() noexcept;
    };
}

#endif