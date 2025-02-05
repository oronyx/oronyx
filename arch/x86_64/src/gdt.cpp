#include "../include/gdt.hpp"

namespace onx
{
    alignas(16) GDTEntry gdt[] = {
        { 0, 0, 0, 0, 0, 0, 0 },
        { 0xFFFF, 0, 0, 0x9A, 0xF, 0xA, 0 },
        { 0xFFFF, 0, 0, 0x92, 0xF, 0xA, 0 }
    };

    GDTDescriptor gdtr = {
        sizeof(gdt) - 1,
        reinterpret_cast<uint64_t>(gdt)
    };
    
    void init_gdt()
    {
        asm volatile("lgdt %0" : : "m"(gdtr));

        // reload the CS
        asm volatile(
            "pushq $0x08\n" // code segment selector
            "pushq $1f\n" // ret addr
            "lretq\n"
            "1:\n"
        );

        // reload data segments
        asm volatile(
            "mov $0x10, %%ax\n"
            "mov %%ax, %%ds\n"
            "mov %%ax, %%es\n"
            "mov %%ax, %%fs\n"
            "mov %%ax, %%gs\n"
            "mov %%ax, %%ss\n"
            : : : "ax"
        );
    }
}