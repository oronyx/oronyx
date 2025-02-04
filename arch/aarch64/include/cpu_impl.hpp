#pragma once

#ifndef __ORNYX__CPU__IMPL__
#define __ORNYX__CPU__IMPL__

#include <ornyx/arch/cpu.hpp>

namespace onx 
{
    template<>
    struct cpu_traits<aarch64> 
    {
        static void enable_interrupts() noexcept;

        static void disable_interrupts() noexcept;

        static void init_cpu() noexcept;
        
        [[noreturn]] static void halt() noexcept;
    };
}

#endif
