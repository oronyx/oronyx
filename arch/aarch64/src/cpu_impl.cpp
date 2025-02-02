#include "../include/cpu_impl.hpp"

namespace onx 
{
    void cpu_traits<aarch64>::enable_interrupts() noexcept 
    {
        asm volatile("msr daifclr, #2");
    }

    void cpu_traits<aarch64>::disable_interrupts() noexcept 
    {
        asm volatile("msr daifset, #2");
    }

    /* TODO: EVT */
    void cpu_traits<aarch64>::init_cpu() noexcept 
    {
        
    }

    [[noreturn]] void cpu_traits<aarch64>::halt() noexcept 
    {
        while(true)
            asm volatile("wfe");
    }
}