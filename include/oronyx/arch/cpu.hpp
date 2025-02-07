#pragma once

#ifndef __ORNYX__CPU__
#define __ORNYX__CPU__

#include <types.hpp>
#include <oronyx/boot/limine.h>

/* CPU-specific stuff that will be in arch/${ARCH} */
namespace onx
{
    struct CPUFeatures;

    template<typename Arch>
    struct cpu_traits
    {
        static void enable_interrupts() noexcept;

        static void disable_interrupts() noexcept;

        [[noreturn]] static void halt() noexcept;

        static void init(volatile limine_smp_request *mp, volatile limine_hhdm_request* hhdm);

        static CPUFeatures& get_features() noexcept;

        static uint64_t get_tick() noexcept;

        template<typename F>
        static void set_gate(uint8_t vector, F handler, uint8_t ist = 0, uint8_t dpl = 0) noexcept;
    };

    using cpu = cpu_traits<current_arch>;
}

#endif