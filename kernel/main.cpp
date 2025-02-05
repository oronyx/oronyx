#include <ornyx/textmode.hpp>
#include <ornyx/arch/cpu.hpp>
#include <ornyx/arch/mem.hpp>
#include <ornyx/boot/limine.h>
#include <ornyx/arch/paging.hpp>

/*
 * limine boot sequences
 * - framebuffer
 * - processor
 * - hhdm
 * - memmap
 */
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

    __attribute__((used, section(".limine_requests")))
    volatile limine_smp_request mp_request = {
        .id = LIMINE_SMP_REQUEST,
        .revision = 0,
        .response = nullptr,
        .flags = 1 /* enable if x2APIC is possible */
    };

    __attribute__((used, section(".limine_requests")))
    inline volatile limine_hhdm_request hhdm_request = {
        .id = LIMINE_HHDM_REQUEST,
        .revision = 0,
        .response = nullptr
    };

    __attribute__((used, section(".limine_requests")))
    inline volatile limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .response = nullptr
    };

    __attribute__((used, section(".limine_requests")))
    volatile limine_paging_mode_request paging_request = {
        .id = LIMINE_PAGING_MODE_REQUEST,
        .revision = 1,
        .response = nullptr,

        /* these are known at compile-time */
        .mode = onx::paging::default_mode(),
        .max_mode = onx::paging::max_mode(),
        .min_mode = onx::paging::min_mode()
    };

    __attribute__((used, section(".limine_requests_start")))
    volatile LIMINE_REQUESTS_START_MARKER;

    __attribute__((used, section(".limine_requests_end")))
    volatile LIMINE_REQUESTS_END_MARKER;
}

/* our entry point */
extern "C" void kernel_main()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
        onx::cpu::halt();

    /* 
     * getting text mode display driver
     * register, and then set it as the current
     * then pass the framebuffer to the driver(?)
     */
    if (framebuffer_requests.response == nullptr ||
        framebuffer_requests.response ->framebuffer_count < 1
    )
    {
        onx::cpu::halt();
    }

    const auto fb = framebuffer_requests.response->framebuffers[0];
    onx::textmode::init(fb);

    /* test init: gradient */
    /*
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
    */
    /* test end: gradient */

    onx::mem::init(&hhdm_request, &memmap_request);
    /*
     * **TODO**:
     *  boot process
     * - [ ] mm init <- priority
     * - [X] cpu init <- handled by limine
     * - [X] idt init <- handled in the cpu init
     * - [ ] apit init
     * - [ ] paging init
     * - [ ] syscall init
     * - [ ] scheduling init
    */

    onx::textmode::write("\n");
    onx::textmode::write("\t\n");
    onx::textmode::write_line("WELCOME TO ORNYX");
    onx::textmode::write_line("RUNNING ON x86_64");
    onx::cpu::init(&mp_request, &hhdm_request);

    /* everything else here should be thread-safe */
    // MAYBE-TODO: panic("task scheduling failed");
    //  maybe retry (?)
    /*
     * do nothing because if we reached this point
     * in a real boot, we are absolutely cooked
     */
    onx::cpu::halt(); /* '::halt' internally is yielding with a loop */
}
