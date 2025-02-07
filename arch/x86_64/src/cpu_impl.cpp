#include "../include/cpu_impl.hpp"
#include <iostream.hpp>
#include <type_traits.hpp>
#include <ornyx/arch/io.hpp>
#include <ornyx/arch/mem.hpp>
#include <ornyx/boot/limine.h>
#include <ornyx/drivers/graphics.hpp>
#include <ornyx/drivers/keyboard.hpp>
#include "../include/mem_impl.hpp"

namespace onx
{
    /* 64-bit GDT */
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

    /* TSS */
    struct TSS
    {
        uint32_t reserved0;
        uint64_t rsp[3]; // rsp0-3
        uint64_t reserved1;
        uint64_t ist[7]; // ist1-7
        uint64_t reserved2;
        uint16_t reserved3;
        uint16_t iopb;
    } __attribute__((packed));



    /* interrupts */
    struct IDTEntry
    {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t ist;
        uint8_t flags;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t reserved;
    } __attribute__((packed));

    struct IDTDescriptor
    {
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed));

    struct InterruptFrame
    {
        uint64_t ip;
        uint64_t cs;
        uint64_t flags;
        uint64_t sp;
        uint64_t ss;
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

    static constexpr uint8_t EXCEPTION_DE = 0;  // divide error
    static constexpr uint8_t EXCEPTION_DB = 1;  // debug
    static constexpr uint8_t EXCEPTION_NMI = 2; // non-maskable int
    static constexpr uint8_t EXCEPTION_BP = 3;  // breakpoint
    static constexpr uint8_t EXCEPTION_OF = 4;  // overflow
    static constexpr uint8_t EXCEPTION_BR = 5;  // bound Range
    static constexpr uint8_t EXCEPTION_UD = 6;  // invalid opcode
    static constexpr uint8_t EXCEPTION_NM = 7;  // device unavailable
    static constexpr uint8_t EXCEPTION_DF = 8;  // double fault     (Error Code)
    static constexpr uint8_t EXCEPTION_CSO = 9; // coprocessor segment Ooerrun
    static constexpr uint8_t EXCEPTION_TS = 10; // invalid TSS      (Error Code)
    static constexpr uint8_t EXCEPTION_NP = 11; // segment not present (Error Code)
    static constexpr uint8_t EXCEPTION_SS = 12; // stack fault      (Error Code)
    static constexpr uint8_t EXCEPTION_GP = 13; // general protection (Error Code)
    static constexpr uint8_t EXCEPTION_PF = 14; // page eault       (Error Code)
    static constexpr uint8_t EXCEPTION_MF = 16; // x87 FPU error
    static constexpr uint8_t EXCEPTION_AC = 17; // alignment check  (Error Code)
    static constexpr uint8_t EXCEPTION_MC = 18; // machine check
    static constexpr uint8_t EXCEPTION_XM = 19; // SIMD exception
    static constexpr uint8_t EXCEPTION_VE = 20; // virtualization exception
    static constexpr uint8_t IRQ_TIMER = 32;    // PIT/APIC Timer
    static constexpr uint8_t IRQ_KEYBOARD = 33; // keyboard
    static constexpr uint8_t IRQ_CASCADE = 34;  // cascade for 8259A Slave
    static constexpr uint8_t IRQ_COM2 = 35;     // COM2
    static constexpr uint8_t IRQ_COM1 = 36;     // COM1
    static constexpr uint8_t IRQ_LPT2 = 37;     // LPT2
    static constexpr uint8_t IRQ_FLOPPY = 38;   // floppy
    static constexpr uint8_t IRQ_LPT1 = 39;     // LPT1/unreliable "spurious" interrupt
    static constexpr uint8_t IRQ_RTC = 40;      // CMOS RTC
    static constexpr uint8_t IRQ_9 = 41;        // Free/SCSI/NIC
    static constexpr uint8_t IRQ_10 = 42;       // Free/SCSI/NIC
    static constexpr uint8_t IRQ_11 = 43;       // Free/SCSI/NIC
    static constexpr uint8_t IRQ_MOUSE = 44;    // PS2 Mouse
    static constexpr uint8_t IRQ_FPU = 45;      // FPU/Coprocessor
    static constexpr uint8_t IRQ_ATA1 = 46;     // primary ATA
    static constexpr uint8_t IRQ_ATA2 = 47;     // secondary ATA
    static constexpr uint8_t IRQ_THERMAL = 0x80;
    static constexpr uint8_t IRQ_PERFMON = 0x81;
    static constexpr uint8_t IRQ_LINT0 = 0x82;
    static constexpr uint8_t IRQ_LINT1 = 0x83;
    static constexpr uint8_t IRQ_ERROR = 0x84;

    /* MSR stuff */
    static constexpr auto MSR_EFER = 0xC0000080;
    static constexpr auto MSR_STAR = 0xC0000081;
    static constexpr auto MSR_LSTAR = 0xC0000082;
    static constexpr auto MSR_SYSCALL_MASK = 0xC0000084;
    static constexpr auto MSR_GS_BASE = 0xC0000101;
    static constexpr auto MSR_KERNEL_GS_BASE = 0xC0000102;

    constexpr uint16_t TSS_LIMIT = sizeof(TSS);

    using InterruptHandler = void(*)(InterruptFrame *);
    using ErrorInterruptHandler = void(*)(InterruptFrame *, uint64_t);

    static uint64_t hhdm_offset = 0;
    static volatile uint64_t tick = 0; /* for APIC & scheduler */
    static InterruptHandler handlers[256] = { nullptr };

    alignas(16) GDTEntry gdt[] = {
        { 0, 0, 0, 0, 0, 0, 0 }, // null
        { 0xFFFF, 0, 0, 0x9A, 0xF, 0xA, 0 }, // kernel code
        { 0xFFFF, 0, 0, 0x92, 0xF, 0xA, 0 }, // kernel data
        { 0, 0, 0, 0, 0, 0, 0 }, // tss low
        { 0, 0, 0, 0, 0, 0, 0 }, // tss high
    };

    alignas(16) static TSS tss = {};
    alignas(16) static IDTEntry idt[256];
    static IDTDescriptor idtr = {
        sizeof(idt) - 1,
        reinterpret_cast<uint64_t>(idt)
    };

    GDTDescriptor gdtr = {
        sizeof(gdt) - 1,
        reinterpret_cast<uint64_t>(gdt)
    };

    CPUFeatures cpu_features;

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

    /* for int dispatch */
    __attribute__((interrupt))
    inline void int_no_error(InterruptFrame *frame)
    {
        /* this gets interrupt number from stack & will be pushed by our stub */
        uintptr_t int_no;
        asm volatile(
            "mov 8(%%rbp), %0"
            : "=r"(int_no)
            :
            : "memory"
        );

        if (handlers[int_no])
            handlers[int_no](frame);
    }

    /* for int dispatch */
    __attribute__((interrupt))
    inline void int_with_error(InterruptFrame *frame, uint64_t error)
    {
        uintptr_t int_no;
        asm volatile(
            "mov 16(%%rbp), %0"
            : "=r"(int_no)
            :
            : "memory"
        );

        if (handlers[int_no])
            reinterpret_cast<ErrorInterruptHandler>(handlers[int_no])(frame, error);
    }

    /* for APIC */
    __attribute__((interrupt))
    void timer_handler(InterruptFrame*)
    {
        tick++;
        if (cpu_features.x2apic)
        {
            wrmsr(0x80B, 0);
        }
        else
        {
            volatile auto *apic = reinterpret_cast<volatile uint32_t *>(0xFEE00000 + hhdm_offset);
            apic[0xB0 / 4] = 0;
        }
    }

    __attribute__((interrupt))
    void page_fault_handler(InterruptFrame *frame, uint64_t error)
    {
        /* typically, cr2 contains the faulting address */
        uint64_t fault_addr;
        asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
        /*
          err code:
            P (bit 0) - 0: non-present page, 1: protection violation
            W (bit 1) - 0: read, 1: write
            U (bit 2) - 0: kernel, 1: user
            R (bit 3) - 0: not reserved bit, 1: reserved bit violation
            I (bit 4) - 0: not instruction fetch, 1: instruction fetch
        */
        write("Page Fault at 0x");
        print_hex(fault_addr);
        write(" (");
        if (error & (1 << 0))
            write("protection violation, ");
        else
            write("non-present page, ");
        if (error & (1 << 1))
            write("write, ");
        else
            write("read, ");
        if (error & (1 << 2))
            write("user, ");
        else
            write("kernel, ");
        write_line(")");

        cpu::halt(); /* TODO: proper fault handling */
    }

    /* general fault */
    __attribute__((interrupt))
    void gpf_handler(const InterruptFrame* frame, const uint64_t error)
    {
        write("General Protection Fault! Error code: ");
        print_hex(error);
        write(" at RIP: ");
        print_hex(frame->ip);
        write_line("");
        cpu::halt();
    }

    __attribute__((interrupt))
    void double_fault_handler(const InterruptFrame *frame, uint64_t)
    {
        write_line("DOUBLE FAULT!");
        write("At RIP: ");
        print_hex(frame->ip);
        write_line("");
        cpu::halt(); /* we halt because a double fault means the OS is cooked */
    }

    template<typename T>
    using BaseInterruptHandler = void(*)(T*);

    template<typename T>
    using BaseErrorInterruptHandler = void(*)(T*, uint64_t);

    using InterruptHandler = BaseInterruptHandler<InterruptFrame>;
    using ErrorInterruptHandler = BaseErrorInterruptHandler<InterruptFrame>;

    template<typename HandlerType>
    void setup_idt(const uint8_t vector, HandlerType *handler, const uint16_t selector = 0x08, const uint8_t ist = 0)
    {
        const auto handler_addr = reinterpret_cast<uintptr_t>(handler);
        IDTEntry &entry = idt[vector];
        entry.offset_low = handler_addr & 0xFFFF;
        entry.selector = selector;
        entry.ist = ist;
        entry.flags = 0x8E;
        entry.offset_mid = (handler_addr >> 16) & 0xFFFF;
        entry.offset_high = (handler_addr >> 32) & 0xFFFFFFFF;

        using NoErrorHandlerType = BaseInterruptHandler<const InterruptFrame>;
        using ErrorHandlerType = BaseErrorInterruptHandler<const InterruptFrame>;

        if constexpr (is_same<HandlerType*, NoErrorHandlerType>::value ||
                      is_same<HandlerType*, ErrorHandlerType>::value ||
                      is_same<HandlerType*, BaseInterruptHandler<InterruptFrame>>::value ||
                      is_same<HandlerType*, BaseErrorInterruptHandler<InterruptFrame>>::value)
        {
            handlers[vector] = reinterpret_cast<InterruptHandler>(handler);
        }
        else
        {
            static_assert(always_false<HandlerType>, "Unsupported interrupt handler type");
        }
    }

    /* TODO: to be replaced with handlers */
    inline void setup_exc()
    {
        setup_idt(EXCEPTION_DE, int_no_error);   // divide error
        setup_idt(EXCEPTION_DB, int_no_error);   // debug
        setup_idt(EXCEPTION_NMI, int_no_error);  // non-maskable int
        setup_idt(EXCEPTION_BP, int_no_error);   // breakpoint
        setup_idt(EXCEPTION_OF, int_no_error);   // overflow
        setup_idt(EXCEPTION_BR, int_no_error);   // bound rage
        setup_idt(EXCEPTION_UD, int_no_error);   // invalid opcode
        setup_idt(EXCEPTION_NM, int_no_error);   // device unvailable
        setup_idt(EXCEPTION_DF, double_fault_handler); // double fault
        setup_idt(EXCEPTION_CSO, int_no_error);  // coprocessor segment overrun
        setup_idt(EXCEPTION_TS, int_with_error); // invalid TSS
        setup_idt(EXCEPTION_NP, int_with_error); // segment Not present
        setup_idt(EXCEPTION_SS, int_with_error); // stack fault
        setup_idt(EXCEPTION_GP, gpf_handler); // general protection
        setup_idt(EXCEPTION_PF, page_fault_handler); // page fault
        setup_idt(EXCEPTION_MF, int_no_error);   // x87 fPU error
        setup_idt(EXCEPTION_AC, int_with_error); // alignment check
        setup_idt(EXCEPTION_MC, int_no_error);   // machine check
        setup_idt(EXCEPTION_XM, int_no_error);   // SIMD exception
        setup_idt(EXCEPTION_VE, int_no_error);   // virtualization exception
    }

    /* TODO: to be replaced by actual handlers(?) */
    inline void setup_irqs()
    {
        setup_idt(IRQ_TIMER, timer_handler);
        setup_idt(IRQ_KEYBOARD, int_no_error);
        setup_idt(IRQ_CASCADE, int_no_error);
        setup_idt(IRQ_COM2, int_no_error);
        setup_idt(IRQ_COM1, int_no_error);
        setup_idt(IRQ_LPT2, int_no_error);
        setup_idt(IRQ_FLOPPY, int_no_error);
        setup_idt(IRQ_LPT1, int_no_error);
        setup_idt(IRQ_RTC, int_no_error);
        setup_idt(IRQ_9, int_no_error);
        setup_idt(IRQ_10, int_no_error);
        setup_idt(IRQ_11, int_no_error);
        setup_idt(IRQ_MOUSE, int_no_error);
        setup_idt(IRQ_FPU, int_no_error);
        setup_idt(IRQ_ATA1, int_no_error);
        setup_idt(IRQ_ATA2, int_no_error);
    }

    void cpu_traits<x86_64>::enable_interrupt()
    {
        asm volatile("sti");
    }

    void cpu_traits<x86_64>::disable_interrupt()
    {
        asm volatile("cli");
    }

    void cpu_traits<x86_64>::init(volatile limine_smp_request *mp, volatile limine_hhdm_request *hhdm) noexcept
    {
        /* save HHDM offset */
        hhdm_offset = hhdm->response->offset;

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

        /* TSS */
        const auto tss_base = reinterpret_cast<uint64_t>(&tss);

        /* generally TSS descriptor is 16 bytes which is split across 2 GDT entries */
        /* first entry [LOW] */
        gdt[3].limit_low = TSS_LIMIT & 0xFFFF;
        gdt[3].base_low = tss_base & 0xFFFF;
        gdt[3].base_middle = (tss_base >> 16) & 0xFF;
        gdt[3].access = 0x89; // PRESENT | RING0 | TSS
        gdt[3].limit_high = (TSS_LIMIT >> 16) & 0xF;
        gdt[3].flags = 0;
        gdt[3].base_high = (tss_base >> 24) & 0xFF;

        /* second entry [HIGH]; see GDT definitions for more info */
        gdt[4].limit_low = (tss_base >> 32) & 0xFFFF;
        gdt[4].base_low = (tss_base >> 48) & 0xFFFF;
        gdt[4].base_middle = 0;
        gdt[4].access = 0;
        gdt[4].limit_high = 0;
        gdt[4].flags = 0;
        gdt[4].base_high = 0;

        /* load our defined TSS */
        tss.iopb = sizeof(TSS);
        asm volatile(
            "ltr %%ax"
            : : "a"(0x18)  /* according to OSDev wiki; it is at 0x18 (3 * 8) */
        );

        // write_line("TSS initialized");
        /* IDT */
        for (auto &handler: handlers)
            handler = nullptr;

        setup_exc();
        setup_irqs();                          /* please don't forget to get back to this */
        asm volatile("lidt %0" : : "m"(idtr)); // load the IDT

        /* we are not enabling int here because we want to set up */
        /* either APIC or PIC first */
        /* kinda risky but breh */

        /* CPU features */
        // TODO: check vendor
        uint32_t eax, ebx, ecx, edx;
        asm volatile("cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(1)
        );

        cpu_features.apic = (edx & (1 << 9)); /* if apic is supported */
        cpu_features.sse = (edx & (1 << 25)); /* if sse SIMD is supported */
        bool xsave = (ecx & (1 << 26));       /* needed for AVX */
        cpu_features.avx = (ecx & (1 << 28)) && xsave;

        /* check AVX2 and AVX-512 features */
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

            // write_line("SSE supported");
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

            // write_line("AVX supported");
            // if (cpu_features.avx512f)
            //     write_line("AVX-512 supported");
        }

        if (cpu_features.apic)
        {
            /* disable PIC if it's on (idk if limine enables them) */
            io::outb(0xA1, 0xFF);
            io::outb(0x21, 0xFF);
            if (cpu_features.x2apic)
            {
                uint64_t apic_base_msr = rdmsr(0x1B);
                wrmsr(0x1B, apic_base_msr | (1 << 10) | (1 << 11));
                // write_line("x2APIC supported");
            }
            else
            {
                uint64_t apic_base_msr = rdmsr(0x1B);
                wrmsr(0x1B, apic_base_msr | (1 << 11));
                // write_line("Base APIC supported");
            }

            constexpr uintptr_t APIC_BASE = 0xFEE00000;
            mem::map_page(
                APIC_BASE + hhdm->response->offset,
                APIC_BASE,
                PRESENT |
                WRITABLE |
                CACHE_DISABLE
            );

            // map the APIC registers for xAPIC mode
            volatile uint32_t *apic = cpu_features.x2apic
                                          ? nullptr
                                          : reinterpret_cast<volatile uint32_t *>(APIC_BASE + hhdm->response->offset);

            /*
             * [1]: div config
             * [2]: init count
             * [3]: periodic timer vector
             */
            if (cpu_features.x2apic)
            {
                wrmsr(0x832, 0x3);
                wrmsr(0x838, 0x100000);
                wrmsr(0x832, IRQ_TIMER | 0x20000);
            }
            else if (apic)
            {
                apic[0x3E0 / 4] = 0x3;
                apic[0x380 / 4] = 0x100000;
                apic[0x320 / 4] = IRQ_TIMER | 0x20000;
            }

            /*
             * configure LVT entries
             * @note
             *  we're NOT masking them (no 0x10000 bit)
             *
             * [1]: timer
             * [2]: thermal
             * [3]: performance
             * [4]: LINT0
             * [5]: LINT1
             * [6]: error
             */
            if (cpu_features.x2apic)
            {
                wrmsr(0x833, IRQ_TIMER);
                wrmsr(0x834, IRQ_THERMAL);
                wrmsr(0x835, IRQ_PERFMON);
                wrmsr(0x836, IRQ_LINT0);
                wrmsr(0x837, IRQ_LINT1);
                wrmsr(0x834, IRQ_ERROR);

                /* enable APIC and set spurious vector */
                wrmsr(0x80F, 0x100 | 0xFF);
            }
            else if (apic)
            {
                apic[0x320 / 4] = IRQ_TIMER;
                apic[0x330 / 4] = IRQ_THERMAL;
                apic[0x340 / 4] = IRQ_PERFMON;
                apic[0x350 / 4] = IRQ_LINT0;
                apic[0x360 / 4] = IRQ_LINT1;
                apic[0x370 / 4] = IRQ_ERROR;
                apic[0xF0 / 4] = 0x100 | 0xFF;
            }
        }
        else
        {
            /* PIC init */
            // ICW1: start init sequence
            io::outb(0x20, 0x11); // master
            io::outb(0xA0, 0x11); // slave

            // ICW2: vector offset
            io::outb(0x21, 0x20);
            io::outb(0xA1, 0x28);

            // ICW3: master & slave wiring
            io::outb(0x21, 0x04); // tell master there's a slave at IRQ2
            io::outb(0xA1, 0x02); // tell slave its cascade identity

            // ICW4: set x86 mode
            io::outb(0x21, 0x01);
            io::outb(0xA1, 0x01);
            // write_line("Using legacy PIC");
        }

        enable_interrupt();
        // write_line("APIC initialized with interrupts enabled");

        /* enable syscall & sysret */
        uint64_t efer = rdmsr(MSR_EFER);
        efer |= 1ULL << 0;
        wrmsr(MSR_EFER, efer);

        /* STAR & LSTAR setup */
        uint64_t star = (0x13ULL << 48) | (0x08ULL << 32);
        wrmsr(MSR_STAR, star);

        /* syscall entry point */
        static auto syscall_entry = []() __attribute__((naked))
        {
            asm volatile(
                "swapgs\n"
                "movq %rsp, %gs:16\n"
                "movq %gs:8, %rsp\n"
                "ret"
            );
        };
        wrmsr(MSR_LSTAR, reinterpret_cast<uint64_t>(+syscall_entry));
        wrmsr(MSR_SYSCALL_MASK, 0x200);

        /* GC base because this is IMPORTANT to separate kernel/user */
        wrmsr(MSR_GS_BASE, 0);
        wrmsr(MSR_KERNEL_GS_BASE, 0);
        // write_line("MSR enabled");

        /* TODO: SMP */

        write_line("CPU initialized");
    }

    [[noreturn]] void cpu_traits<x86_64>::halt() noexcept
    {
        while (true)
            asm volatile("hlt");
    }

    CPUFeatures& cpu_traits<x86_64>::get_features() noexcept
    {
        return cpu_features;
    }

    uint64_t cpu_traits<x86_64>::get_tick() noexcept
    {
        return tick;
    }
}
