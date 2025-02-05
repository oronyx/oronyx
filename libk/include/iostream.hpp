#pragma once

#include <types.hpp>

namespace onx
{
    void putc(char c);

    void puts(const char* str);

    void putln(const char* str);

    void endl();

    void print_int(size_t value);

    void print_uint(unsigned int value);

    void print_hex(unsigned int value);
}