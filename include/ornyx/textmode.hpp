#pragma once

#ifndef __ORNYX__TEXTMODE__
#define __ORNYX__TEXTMODE__

#include <types.hpp>
#include <ornyx/boot/limine.h>

namespace onx::textmode
{
    static constexpr size_t BASIC_WIDTH = 8;  // latin
    static constexpr size_t WIDE_WIDTH = 16;  // cjk
    static constexpr size_t FONT_HEIGHT = 16;

    void init(limine_framebuffer* framebuffer) noexcept;

    void clear() noexcept;

    void putchar(char c) noexcept;

    void write(const char* str) noexcept;

    void write_line(const char* str) noexcept;

    void setcolor(uint32_t fg, uint32_t bg) noexcept;
    
    void setcursor(size_t x, size_t y) noexcept;

    size_t get_x() noexcept;

    size_t get_y() noexcept;

    size_t get_width() noexcept;

    size_t get_height() noexcept;
}

#endif