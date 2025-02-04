#include <ornyx/boot/init.hpp>

extern void (*__init_array_start[])() __attribute__((weak));
extern void (*__init_array_end[])() __attribute__((weak));

namespace onx
{
    /* global constructors */
    void init_globals() noexcept
    {
        for (void (**ctor)() = __init_array_start; ctor < __init_array_end; ++ctor)
            (*ctor)();
    }
}