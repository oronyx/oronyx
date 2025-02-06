#include "../include/io_impl.hpp"

namespace onx
{
    uint8_t io_traits<x86_64>::inb(uint16_t port) noexcept
    {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
        return ret;
    }

    void io_traits<x86_64>::outb(uint16_t port, uint8_t val) noexcept
    {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
    }

    void io_traits<x86_64>::outw(uint16_t port, uint16_t val) noexcept
    {
        asm volatile("outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
    }
}