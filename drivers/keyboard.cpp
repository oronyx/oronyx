#include <ornyx/arch/io.hpp>
#include <ornyx/drivers/keyboard.hpp>

namespace onx
{
    struct InterruptFrame
    {
        uint64_t ip;
        uint64_t cs;
        uint64_t flags;
        uint64_t sp;
        uint64_t ss;
    } __attribute__((packed));

    static char last_key;
    static bool keys[128] = { false };
    static auto shift_pressed = false;
    static auto caps_lock = false;

    /* TODO: different layout/translation */
    static constexpr char SCANCODE2ASCII[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };

    static constexpr char SCANCODE2ASCII_SHIFT[] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
        0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
        0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
        '*', 0, ' '
    };

    char get_last_key() noexcept
    {
        return last_key;
    }

    bool is_pressed(const uint8_t scancode) noexcept
    {
        return keys[scancode & 0x7F];
    }

    char translate_scancode(const uint8_t scancode) noexcept
    {
        if (scancode >= sizeof(SCANCODE2ASCII))
            return 0;

        const bool shift = shift_pressed ^ caps_lock;
        return shift ? SCANCODE2ASCII_SHIFT[scancode] : SCANCODE2ASCII[scancode];
    }

    __attribute__((interrupt))
    void keyboard_handler(InterruptFrame *) noexcept
    {
        uint8_t scancode = io::inb(0x60);
        const bool released = scancode & 0x80;
        scancode &= 0x7F;

        switch (scancode)
        {
            case 0x2A: /* left shift */
            case 0x36: /* right shift */
                shift_pressed = !released;
                return;
            case 0x3A: /* caps lock */
                if (!released)
                    caps_lock = !caps_lock;
                return;
            default:
            {
                keys[scancode] = !released;
                if (!released)
                {
                    last_key = translate_scancode(scancode);
                }
            }
        }
    }
}
