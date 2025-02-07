#include "../include/mem_impl.hpp"

#include <iostream.hpp>
#include <oronyx/arch/cpu.hpp>
#include <oronyx/boot/limine.h>
#include <oronyx/drivers/graphics.hpp>
#include <oronyx/mm/pmm.hpp>

namespace onx
{
    static uint64_t hhdm_offset { 0 };

    uint64_t *mm_trait<x86_64>::alloc_page() noexcept
    {
        const uintptr_t phys_addr = mm::PMM::alloc_pages(1);
        if (!phys_addr)
        {
            write_line("Failed to allocate physical page");
            return nullptr;
        }

        return reinterpret_cast<uint64_t *>(phys_addr + hhdm_offset);
    }

    void mm_trait<x86_64>::flush_tlb(void *addr) noexcept
    {
        asm volatile("invlpg (%0)" :: "r"(addr) : "memory");
    }

    void mm_trait<x86_64>::init(volatile limine_hhdm_request *hhdm,
                                volatile limine_memmap_request *mmap) noexcept
    {
        if (!hhdm->response || !mmap->response)
        {
            putln("MM init failed: no HHDM or memmap response");
            return;
        }

        hhdm_offset = hhdm->response->offset;

        uint64_t cr3;
        asm volatile("mov %%cr3, %0" : "=r"(cr3));

        mm::PMM::init(mmap, hhdm_offset);
        if (uint64_t *phys_addr = alloc_page();
            !phys_addr)
        {
            putln("Failed to allocate physical page");
            cpu::halt();
        }

        /* security */
        putln("Enabling NX bit...");
        uint64_t efer;
        asm volatile("rdmsr" : "=A"(efer) : "c"(0xC0000080));
        efer |= 1ULL << 11;
        asm volatile("wrmsr" : : "c"(0xC0000080), "A"(efer));
        putln("Setting WP bit...");
        asm volatile(
            "mov %%cr0, %%rax\n"
            "or $0x10000, %%rax\n"
            "mov %%rax, %%cr0"
            ::: "rax", "memory"
        );
        putln("MM initialized");
    }

    void mm_trait<x86_64>::map_page(uintptr_t virt, uintptr_t phys, uint64_t flags) noexcept
    {
        const size_t pml4_index = (virt >> 39) & 0x1FF;
        const size_t pdpt_index = (virt >> 30) & 0x1FF;
        const size_t pd_index = (virt >> 21) & 0x1FF;
        const size_t pt_index = (virt >> 12) & 0x1FF;

        uint64_t cr3;
        asm volatile("mov %%cr3, %0" : "=r"(cr3));

        auto *pml4 = reinterpret_cast<uint64_t *>((cr3 & ~0xFFF) + hhdm_offset);
        if (flags & HUGE)
        {
            uint64_t *pdpt;
            uint64_t *pd;
            if (!(pml4[pml4_index] & PRESENT))
            {
                pdpt = alloc_page();
                if (!pdpt)
                    return;
                pml4[pml4_index] = (reinterpret_cast<uint64_t>(pdpt) - hhdm_offset) | (flags & 0xFFF);
            }
            else
            {
                pdpt = reinterpret_cast<uint64_t *>((pml4[pml4_index] & ~0xFFF) + hhdm_offset);
            }

            if (!(pdpt[pdpt_index] & PRESENT))
            {
                pd = alloc_page();
                if (!pd)
                    return;
                pdpt[pdpt_index] = (reinterpret_cast<uint64_t>(pd) - hhdm_offset) | (flags & 0xFFF);
            }
            else
            {
                pd = reinterpret_cast<uint64_t *>((pdpt[pdpt_index] & ~0xFFF) + hhdm_offset);
            }

            /* map our 2MB page */
            pd[pd_index] = (phys & ~0x1FFFFF) | (flags & 0xFFF) | HUGE;
            flush_tlb(reinterpret_cast<void *>(virt));
            return;
        }

        /* 4KB paging routine */
        uint64_t *pdpt;
        uint64_t *pd;
        uint64_t *pt;
        if (!(pml4[pml4_index] & PRESENT))
        {
            pdpt = alloc_page();
            if (!pdpt)
                return;
            pml4[pml4_index] = (reinterpret_cast<uint64_t>(pdpt) - hhdm_offset) | (flags & 0xFFF);
        }
        else
        {
            pdpt = reinterpret_cast<uint64_t *>((pml4[pml4_index] & ~0xFFF) + hhdm_offset);
        }

        if (!(pdpt[pdpt_index] & PRESENT))
        {
            pd = alloc_page();
            if (!pd)
                return;
            pdpt[pdpt_index] = (reinterpret_cast<uint64_t>(pd) - hhdm_offset) | (flags & 0xFFF);
        }
        else
        {
            pd = reinterpret_cast<uint64_t *>((pdpt[pdpt_index] & ~0xFFF) + hhdm_offset);
        }

        if (!(pd[pd_index] & PRESENT))
        {
            pt = alloc_page();
            if (!pt)
                return;
            pd[pd_index] = (reinterpret_cast<uint64_t>(pt) - hhdm_offset) | (flags & 0xFFF);
        }
        else
        {
            pt = reinterpret_cast<uint64_t *>((pd[pd_index] & ~0xFFF) + hhdm_offset);
        }

        /* map our 4KB page */
        pt[pt_index] = (phys & ~0xFFF) | (flags & 0xFFF);
        flush_tlb(reinterpret_cast<void *>(virt));
    }
}
