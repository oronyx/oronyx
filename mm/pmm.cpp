#include <iostream.hpp>
#include <ornyx/textmode.hpp>
#include <ornyx/arch/cpu.hpp>
#include <ornyx/mm/pmm.hpp>

namespace onx::mm
{
    /* TODO: this will be turned into a bitmap */
    inline Region regions[MAX_REGIONS];
    size_t region_count = 0;
    static uint64_t hhdm_offset = 0;

    void PMM::init(const volatile limine_memmap_request *mmap, const uint64_t offset) noexcept
    {
        if (!mmap->response)
        {
            putln("PMM init failed: no memmap response");
            return;
        }

        hhdm_offset = offset;
        for (size_t i = 0; i < mmap->response->entry_count && region_count < MAX_REGIONS; i++)
        {
            if (const auto entry = mmap->response->entries[i];
                entry->type == LIMINE_MEMMAP_USABLE)
            {
                /* align base and len with page boundaries */
                const uintptr_t base = (entry->base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
                if (const uintptr_t end = (entry->base + entry->length) & ~(PAGE_SIZE - 1);
                    end > base)
                {
                    regions[region_count++] = {
                        .base = base,
                        .len = end - base,
                        .is_free = true
                    };
                }
            }
        }

        /* sort regions by base address */
        /* so inefficient...bubble sort  */
        for (size_t i = 0; i < region_count - 1; ++i)
        {
            for (size_t j = 0; j < region_count - i - 1; ++j)
            {
                if (regions[j].base > regions[j + 1].base)
                {
                    const auto temp = regions[j];
                    regions[j] = regions[j + 1];
                    regions[j + 1] = temp;
                }
            }
        }

        /* merge all regions that are adjacent  */
        for (size_t i = 0; i < region_count - 1; i++)
        {
            if (regions[i].base + regions[i].len == regions[i + 1].base) {
                regions[i].len += regions[i + 1].len;
                for (size_t j = i + 1; j < region_count - 1; ++j)
                    regions[j] = regions[j + 1];
                region_count--;
                i--;
            }
        }

        print_regions();
        textmode::write_line("PMM initialized");
    }

    uintptr_t PMM::alloc_pages(const size_t count) noexcept
    {
        /* find the first fit */
        const size_t sz = count * PAGE_SIZE;
        for (size_t i = 0; i < region_count; ++i)
        {
            if (regions[i].is_free && regions[i].len >= sz)
            {
                const uintptr_t alloc_base = regions[i].base;
                if (regions[i].len == sz)
                {
                    regions[i].is_free = false;
                }
                else
                {
                    if (region_count >= MAX_REGIONS)
                    {
                        /* maybe change this to a stylized text with BG */
                        putln("PMM: No space for new region!");
                        return 0;
                    }
                    for (size_t j = region_count; j > i + 1; j--)
                        regions[j] = regions[j - 1];

                    regions[i + 1].base = regions[i].base + sz;
                    regions[i + 1].len = regions[i].len - sz;
                    regions[i + 1].is_free = true;

                    regions[i].len = sz;
                    regions[i].is_free = false;
                    region_count++;
                }

                /*
                 * NOTETOSELF: never forget to account for the actual hhdm offset
                 *  even if it is shown to be zero (for some reason lol)
                 *
                 * TODO: change this to a performant memset()
                 */
                auto* ptr = reinterpret_cast<volatile uint64_t*>(alloc_base + hhdm_offset);
                for (size_t j = 0; j < sz / sizeof(uint64_t); j++)
                    ptr[j] = 0;

                return alloc_base;
            }
        }

        /* maybe change this to a stylized text with BG */
        putln("PMM: No suitable region found!");
        return 0;
    }

    void PMM::free_pages(const uintptr_t base, const size_t count) noexcept
    {
        size_t size = count * PAGE_SIZE;
        for (size_t i = 0; i < region_count; i++)
        {
            if (regions[i].base == base && regions[i].len == size)
            {
                regions[i].is_free = true;

                if (i > 0 && regions[i - 1].is_free)
                {
                    regions[i - 1].len += regions[i].len;
                    for (size_t j = i; j < region_count - 1; j++)
                        regions[j] = regions[j + 1];
                    region_count--;
                    i--;
                }
                if (i < region_count - 1 && regions[i + 1].is_free)
                {
                    regions[i].len += regions[i + 1].len;
                    for (size_t j = i + 1; j < region_count - 1; j++)
                        regions[j] = regions[j + 1];
                    region_count--;
                }
                return;
            }
        }

        putln("PMM: Tried to free invalid pages!");
    }

    void PMM::print_regions() noexcept
    {
        putln("Physical Memory Regions:");
        for (size_t i = 0; i < region_count; i++)
        {
            puts("  Region ");
            print_int(i);
            puts(": Base=0x");
            print_hex(regions[i].base);
            puts(" Len=0x");
            print_hex(regions[i].len);
            puts(" ");
            puts(regions[i].is_free ? "Free" : "Used");
            endl();
        }
    }

    size_t PMM::get_free_memory() noexcept
    {
        size_t total = 0;
        for (size_t i = 0; i < region_count; i++)
        {
            if (regions[i].is_free)
                total += regions[i].len;
        }
        return total;
    }
}