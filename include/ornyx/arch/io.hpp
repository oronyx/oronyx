#pragma once

#ifndef __ORNYX__IO__
#define __ORNYX__IO__

#include <types.hpp>

namespace onx
{
    template<typename Arch>
    struct io_traits
    {
        static void outb(uint16_t port, uint8_t val) noexcept;
        static void outw(uint16_t port, uint16_t val) noexcept;
        static uint8_t inb(uint16_t port) noexcept;
    };

    using io = io_traits<current_arch>;
}

#endif