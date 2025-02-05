#pragma once

#ifndef __ORNYX__PAGING__IMPL__
#define __ORNYX__PAGING__IMPL__

#include <ornyx/arch/paging.hpp>

template<>
struct paging_trait<x86_64>
{
    static constexpr uint64_t default_mode() noexcept
    {
        return LIMINE_PAGING_MODE_X86_64_4LVL;
    }

    static constexpr uint64_t max_mode() noexcept
    {
        return LIMINE_PAGING_MODE_X86_64_5LVL;
    }

    static constexpr uint64_t min_mode() noexcept
    {
        return LIMINE_PAGING_MODE_X86_64_4LVL;
    }
};

#endif