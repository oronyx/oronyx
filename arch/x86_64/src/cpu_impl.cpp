#include "../include/cpu_impl.hpp"
#include <ornyx/textmode.hpp>
#include <ornyx/boot/limine.h>

namespace onx
{
    struct GDTEntry
    {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t limit_high: 4;
        uint8_t flags: 4;
        uint8_t base_high;
    } __attribute__((packed));

    struct GDTDescriptor
    {
        uint16_t size;
        uint64_t offset;
    } __attribute__((packed));

    /* port & SIMD features */
    struct CPUFeatures
    {
        bool apic { false };
        bool x2apic { false };
        bool sse { false };
        bool avx { false };
        bool avx2 { false };
        bool avx512f { false };
        bool avx512bw { false };
        bool avx512cd { false };
        bool avx512dq { false };
        uint32_t bsp_lapic_id { 0 };
    };

    inline void outb(uint16_t port, uint8_t val)
    {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
    }

    inline void outw(uint16_t port, uint16_t val)
    {
        asm volatile("outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
    }

    inline uint8_t inb(uint16_t port)
    {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
        return ret;
    }

    inline uint64_t rdmsr(uint32_t msr)
    {
        uint32_t low, high;
        asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
        return (static_cast<uint64_t>(high) << 32) | low;
    }

    inline void wrmsr(uint32_t msr, uint64_t value)
    {
        uint32_t low = value & 0xFFFFFFFF;
        uint32_t high = value >> 32;
        asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
    }

    alignas(16) GDTEntry gdt[] = {
        { 0, 0, 0, 0, 0, 0, 0 },
        { 0xFFFF, 0, 0, 0x9A, 0xF, 0xA, 0 },
        { 0xFFFF, 0, 0, 0x92, 0xF, 0xA, 0 }
    };

    GDTDescriptor gdtr = {
        sizeof(gdt) - 1,
        reinterpret_cast<uint64_t>(gdt)
    };

    CPUFeatures cpu_features;

    void cpu_traits<x86_64>::enable_interrupts() noexcept
    {
        asm volatile("sti");
    }

    void cpu_traits<x86_64>::disable_interrupts() noexcept
    {
        asm volatile("cli");
    }

    /* TODO: IDT, MSR x86 */
    void cpu_traits<x86_64>::init(volatile limine_smp_request *mp) noexcept
    {
        /* CPU features */
        uint32_t eax, ebx, ecx, edx;
        asm volatile("cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(1)
        );

        cpu_features.apic = (edx & (1 << 9)); /* if apic is supported */
        cpu_features.sse = (edx & (1 << 25)); /* if sse SIMD is supported */
        bool xsave = (ecx & (1 << 26));       /* needed for AVX */
        cpu_features.avx = (ecx & (1 << 28)) && xsave;

        /* Check AVX2 and AVX-512 features */
        asm volatile("cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(7), "c"(0)
        );

        cpu_features.avx2 = (ebx & (1 << 5));
        if (cpu_features.avx)
        {
            cpu_features.avx512f = (ebx & (1 << 16));  /* foundation */
            cpu_features.avx512dq = (ebx & (1 << 17)); /* doubleword and quadword */
            cpu_features.avx512cd = (ebx & (1 << 28)); /* conflict detection */
            cpu_features.avx512bw = (ebx & (1 << 30)); /* byte and word */
        }

        if (mp && mp->response)
        {
            auto *smp_info = mp->response;
            cpu_features.bsp_lapic_id = smp_info->bsp_lapic_id;
            cpu_features.x2apic = smp_info->flags & LIMINE_SMP_X2APIC; // check if the bit we requested is present
        }

        /* enable all available features */
        if (cpu_features.sse)
        {
            uint64_t cr0;
            asm volatile("mov %%cr0, %0" : "=r"(cr0));
            cr0 &= ~(1 << 2); // clear EM
            cr0 |= (1 << 1);  // set the MP bit
            asm volatile("mov %0, %%cr0" : : "r"(cr0));

            uint64_t cr4;
            asm volatile("mov %%cr4, %0" : "=r"(cr4));
            cr4 |= 1 << 9;  // set OSFXSR
            cr4 |= 1 << 10; // set OSXMMEXCPT
            asm volatile("mov %0, %%cr4" : : "r"(cr4));

            textmode::write_line("SSE supported");
        }

        if (cpu_features.avx)
        {
            uint64_t xcr0;
            asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
            xcr0 = (static_cast<uint64_t>(edx) << 32) | eax;

            xcr0 |= 7; /* enable SSE + AVX state */

            if (cpu_features.avx512f)
            {
                xcr0 |= 1 << 5; /* enable OPMASK state */
                xcr0 |= 1 << 6; /* enable ZMM hi256 state */
                xcr0 |= 1 << 7; /* enable Hi16 ZMM state */
            }

            eax = xcr0 & 0xFFFFFFFF;
            edx = xcr0 >> 32;
            asm volatile("xsetbv" : : "a"(eax), "d"(edx), "c"(0));

            textmode::write_line("AVX supported");
            if (cpu_features.avx512f)
                textmode::write_line("AVX-512 supported");
        }

        if (cpu_features.apic)
        {
            /* disable PIC if it's on (idk if limine enables them) */
            outb(0xA1, 0xFF);
            outb(0x21, 0xFF);
            if (cpu_features.x2apic)
            {
                uint64_t apic_base_msr = rdmsr(0x1B);
                wrmsr(0x1B, apic_base_msr | (1 << 10) | (1 << 11));
                textmode::write_line("x2APIC supported");
            }
            else
            {
                uint64_t apic_base_msr = rdmsr(0x1B);
                wrmsr(0x1B, apic_base_msr | (1 << 11));
                textmode::write_line("Base APIC supported");
            }

            /*
                @note commented out due to kernel crashing
                    will do once the interrupt handling is properly set up

                // map the APIC registers for xAPIC mode
                volatile uint32_t* apic = cpu_features.x2apic ? nullptr
                    : reinterpret_cast<volatile uint32_t*>(0xFEE00000);


                // enable spurious interrupts
                if (cpu_features.x2apic)
                {
                    wrmsr(0x80F, 0x100 | 0xFF);
                }
                else if (apic)
                {
                    apic[0xF0 / 4] = 0x100 | 0xFF;
                }
            */
        }
        else
        {
            /* PIC init */
            // ICW1: start init sequence
            outb(0x20, 0x11);  // master
            outb(0xA0, 0x11);  // slave

            // ICW2: vector offset
            outb(0x21, 0x20);
            outb(0xA1, 0x28);

            // ICW3: master & slave wiring
            outb(0x21, 0x04);  // tell master there's a slave at IRQ2
            outb(0xA1, 0x02);  // tell slave its cascade identity

            // ICW4: set x86 mode
            outb(0x21, 0x01);
            outb(0xA1, 0x01);
            textmode::write_line("Using legacy PIC");
        }

        /* GDT */
        asm volatile("lgdt %0" : : "m"(gdtr));
        asm volatile(
            "pushq $0x08\n"// code segment selector
            "pushq $1f\n"  // ret addr
            "lretq\n"
            "1:\n"
        );
        asm volatile(
            "mov $0x10, %%ax\n"
            "mov %%ax, %%ds\n"
            "mov %%ax, %%es\n"
            "mov %%ax, %%fs\n"
            "mov %%ax, %%gs\n"
            "mov %%ax, %%ss\n"
            : : : "ax"
        );

        /* IDT */
        // TODO: implement IDT setup

        /* MSR */
    }

    [[noreturn]] void cpu_traits<x86_64>::halt() noexcept
    {
        while (true)
            asm volatile("hlt");
    }
}
