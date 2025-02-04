#include <ornyx/arch/cpu.hpp>
#include <ornyx/boot/init.hpp>
#include <ornyx/boot/limine.h>

/* limine boot sequences */
namespace
{
    __attribute__((used, section(".limine_requests")))
    volatile LIMINE_BASE_REVISION(3);

    __attribute__((used, section(".limine_requests")))
    volatile limine_framebuffer_request framebuffer_requests = {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .revision = 0,
        .response = nullptr
    };

    __attribute__((used, section(".limine_requests_start")))
    volatile LIMINE_REQUESTS_START_MARKER;

    __attribute__((used, section(".limine_requests_end")))
    volatile LIMINE_REQUESTS_END_MARKER;
}

extern "C" void kernel_main()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
        onx::cpu::halt();

    /* 
     * getting text mode display driver
     * register, and then set it as current
     * then pass the framebuffer to the driver(?)
     */

    /*
     * **TODO**:
     *  boot process
     * - [X] cpu init <- handled by limine
     * - [ ] idt init
     * - [ ] hal init
     * - [ ] apic init
     * - [ ] apit init
     * - [ ] paging init
     * - [ ] mm init
     * - [ ] syscall init
     * - [ ] scheduling init
    */
    // MAYBE-TODO: panic("task scheduling failed");
    //  maybe retry (?)
    /*
     * do nothing because if we reached this point
     * in a real boot, we are absolutely cooked
     */
    onx::cpu::halt(); /* '::halt' internally is yielding with a loop */
}
