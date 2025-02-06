#pragma once

#ifndef __ORNYX__IO__IMPL__
#include <ornyx/arch/io.hpp>

template<>
struct onx::io_traits<x86_64>
{
    static void outb(uint16_t port, uint8_t val) noexcept;

    static void outw(uint16_t port, uint16_t val) noexcept;

    static uint8_t inb(uint16_t port) noexcept;
};

#endif