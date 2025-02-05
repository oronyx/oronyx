#include <iostream.hpp>
#include <ornyx/textmode.hpp>

namespace onx
{
    void putc(const char c)
    {
        onx::textmode::putchar(c);
    }

    void puts(const char *str)
    {
        onx::textmode::write(str);
    }

    void putln(const char *str)
    {
        onx::textmode::write_line(str);
    }

    void endl()
    {
        onx::textmode::write_line("");
    }

    void print_int(size_t value)
    {
        char buffer[32];
        auto i = 0;
        const bool negative = value < 0;
        if (negative)
            value = -value;

        do
        {
            buffer[i++] = '0' + (value % 10);
            value /= 10;
        }
        while (value);

        if (negative)
            buffer[i++] = '-';

        while (i--)
            putc(buffer[i]);
    }

    void print_uint(unsigned int value)
    {
        char buffer[32];
        auto i = 0;

        do
        {
            buffer[i++] = '0' + (value % 10);
            value /= 10;
        }
        while (value);

        while (i--)
            putc(buffer[i]);
    }

    void print_hex(unsigned int value)
    {
        // puts("0x");
        char buffer[32];
        auto i = 0;

        do
        {
            const uint8_t digit = value & 0xF;
            buffer[i++] = digit < 10 ? '0' + digit : 'a' + (digit - 10);
            value >>= 4;
        }
        while (value);

        while (i--)
            putc(buffer[i]);
    }

    void print_ptr(const void *ptr)
    {
        print_hex(reinterpret_cast<uintptr_t>(ptr));
    }
}
