#include <ornyx/boot/limine.h>
#include <ornyx/arch/cpu.hpp>
#include <ornyx/boot/init.hpp>
#include <ornyx/textmode.hpp>
#include <types.hpp>

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
    if (framebuffer_requests.response == nullptr ||
        framebuffer_requests.response ->framebuffer_count < 1
    )
    {
        onx::cpu::halt();
        return;
    }

    auto fb = framebuffer_requests.response->framebuffers[0];
    onx::textmode::init(fb);
    // onx::cpu::halt();
    /* test init: gradient */
    uint32_t* buffer = (uint32_t*)fb->address;
    size_t pitch = fb->pitch / sizeof(uint32_t);
    for (size_t y = 0; y < fb->height; y++) 
    {
        uint8_t blue = (y * 255) / fb->height;
        for (size_t x = 0; x < fb->width; x++) 
        {
            uint8_t red = (x * 255) / fb->width;
            uint32_t color = (0xFF << 24) | (red << 16) | blue;
            buffer[y * pitch + x] = color;
        }
    }
    /* test end: gradient */

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
    onx::cpu::init_cpu();
    onx::textmode::write_line("WELCOME TO ORNYX");
    onx::textmode::write_line("CPU INITIALIZED");
    onx::textmode::write("RUNNING ON x86_64");

    // MAYBE-TODO: panic("task scheduling failed");
    //  maybe retry (?)
    /*
     * do nothing because if we reached this point
     * in a real boot, we are absolutely cooked
     */
    onx::cpu::halt(); /* '::halt' internally is yielding with a loop */
}
