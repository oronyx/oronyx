#pragma once

#ifndef __ORNYX__CPU__IMPL__
#define __ORNYX__CPU__IMPL__

#include <oronyx/arch/cpu.hpp>

namespace onx
{
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

    template<>
    struct cpu_traits<x86_64>
    {
        static void enable_interrupt() noexcept;

        static void disable_interrupt() noexcept;

        static void init(volatile limine_smp_request *mp, volatile limine_hhdm_request* hhdm) noexcept;

        [[noreturn]] static void halt() noexcept;

        static CPUFeatures& get_features() noexcept;

        static uint64_t get_tick() noexcept;

        template<typename F>
        static void set_gate(uint8_t vector, F handler, uint8_t ist = 0, uint8_t dpl = 0) noexcept;
    };
}

#endif
