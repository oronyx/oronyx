#pragma once

#ifndef __ORNYX__PAGING__
#define __ORNYX__PAGING__

#include <ornyx/boot/limine.h>

namespace onx
{
    template<typename Arch>
    struct paging_trait
    {
        static constexpr uint64_t default_mode() noexcept;
        static constexpr uint64_t max_mode() noexcept;
        static constexpr uint64_t min_mode() noexcept;
    };

    struct x86_64;
    struct aarch64;

#if defined(__x86_64__)
    using current_arch = x86_64;
#include "../../arch/x86_64/include/paging_impl.hpp"
#elif defined(__aarch64__)
    using current_arch = aarch64;
#else
#error "Unsupported architecture"
#endif

    using paging = paging_trait<current_arch>;
}

#endif