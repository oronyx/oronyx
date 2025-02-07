#pragma once

#ifndef __ORNYX__KEYBOARD__
#define __ORNYX__KEYBOARD__

#include <types.hpp>

namespace onx
{
    struct InterruptFrame;

    char get_last_key() noexcept;

    bool is_pressed(uint8_t scancode) noexcept;

    __attribute__((interrupt))
    void keyboard_handler(InterruptFrame *) noexcept;
}

#endif