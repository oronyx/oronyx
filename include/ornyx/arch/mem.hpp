#pragma once

#ifndef __ORNYX__MEM__
#define __ORNYX__MEM__

#include <ornyx/boot/limine.h>
#include <types.hpp>

namespace onx
{
    template<typename Arch>
    struct mm_trait
    {
        static void flush_tlb(void* addr) noexcept;

        static void init(volatile limine_hhdm_request* hhdm,
                        volatile limine_memmap_request* mmap) noexcept;

        static void map_page(uintptr_t virt, uintptr_t phys, uint64_t flags) noexcept;
    };

    using mem = mm_trait<current_arch>;
}

#endif