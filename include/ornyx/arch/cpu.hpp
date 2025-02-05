#pragma once

#ifndef __ORNYX__CPU__
#define __ORNYX__CPU__
#include <ornyx/boot/limine.h>

/* CPU-specific stuff that will be in arch/${ARCH} */
namespace onx
{
    template<typename Arch>
    struct cpu_traits
    {
        static void enable_interrupts() noexcept = delete;
        static void disable_interrupts() noexcept = delete;
        [[noreturn]] static void halt() noexcept;
        static void init(volatile limine_smp_request* mp);
    };

    struct x86_64;
    struct aarch64;

    /* TODO: support more architecture later */
    #if defined(__x86_64__)
        using current_arch = x86_64;
    #elif defined(__aarch64__)
        using current_arch = aarch64;
    #else
        #error "Unsupported architecture"
    #endif

    using cpu = cpu_traits<current_arch>;
}

#endif