#pragma once

#ifndef __ORNYX__CPU__IMPL__
#define __ORNYX__CPU__IMPL__

#include <ornyx/arch/cpu.hpp>

template<>
struct onx::cpu_traits<x86_64>
{
    static void enable_interrupts() noexcept;

    static void disable_interrupts() noexcept;

    static void init(volatile limine_smp_request *mp, volatile limine_hhdm_request* hhdm) noexcept;

    [[noreturn]] static void halt() noexcept;

    static CPUFeatures& get_features() noexcept;

    static uint64_t get_tick() noexcept;
};

#endif
