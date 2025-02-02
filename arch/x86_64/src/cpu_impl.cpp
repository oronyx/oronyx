#include "../include/cpu_impl.hpp"
#include "../include/idt.hpp"

namespace onx 
{
    void cpu_traits<x86_64>::enable_interrupts() noexcept 
    {
        asm volatile("sti");
    }

    void cpu_traits<x86_64>::disable_interrupts() noexcept 
    {
        asm volatile("cli");
    }

    /* TODO: IDT, MSR x86 */
    void cpu_traits<x86_64>::init_cpu() noexcept 
    {
        // IDT
        onx::init_idt();
        // MSR
    }

    [[noreturn]] void cpu_traits<x86_64>::halt() noexcept 
    {
        while(true)
            asm volatile("hlt");
    }
}