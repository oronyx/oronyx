#include <oronyx/arch/io.hpp>
#include <oronyx/drivers/graphics.hpp>
#include <oronyx/drivers/keyboard.hpp>

namespace onx
{
    static constexpr size_t MAX_KEYBOARD_EVENTS = 64;

    struct KeyboardEvent
    {
        char ascii;
        uint8_t scancode;
        bool pressed;
    };

    static struct
    {
        char last_key;
        bool keys[128] = { false };
        bool shift_left = false;
        bool shift_right = false;
        bool caps_lock = false;

        /* circular buffer */
        KeyboardEvent event_queue[MAX_KEYBOARD_EVENTS];
        size_t queue_head = 0;
        size_t queue_tail = 0;
        size_t queue_size = 0;
    } keyboard_state;

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

    static void enqueue_event(char ascii, uint8_t scancode)
    {
        if (keyboard_state.queue_size < MAX_KEYBOARD_EVENTS)
        {
            keyboard_state.event_queue[keyboard_state.queue_tail] = { ascii, scancode, true };
            keyboard_state.queue_tail = (keyboard_state.queue_tail + 1) % MAX_KEYBOARD_EVENTS;
            keyboard_state.queue_size++;
        }
    }

    static char translate_scancode(const uint8_t scancode) noexcept
    {
        if (scancode >= sizeof(SCANCODE2ASCII))
            return 0;

        const bool shift = (keyboard_state.shift_left || keyboard_state.shift_right) ^ keyboard_state.caps_lock;
        return shift ? SCANCODE2ASCII_SHIFT[scancode] : SCANCODE2ASCII[scancode];
    }

    char get_last_key() noexcept
    {
        return keyboard_state.last_key;
    }

    bool is_pressed(const uint8_t scancode) noexcept
    {
        return keyboard_state.keys[scancode & 0x7F];
    }

    bool dequeue_keyboard_event(KeyboardEvent &event)
    {
        if (keyboard_state.queue_size == 0)
            return false;

        event = keyboard_state.event_queue[keyboard_state.queue_head];
        keyboard_state.queue_head = (keyboard_state.queue_head + 1) % MAX_KEYBOARD_EVENTS;
        keyboard_state.queue_size--;
        return true;
    }

    void process_keyboard_input()
    {
        KeyboardEvent event;
        while (dequeue_keyboard_event(event))
        {
            if (event.pressed && event.ascii)
            {
                putchar(event.ascii);
            }
        }
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
                keyboard_state.shift_left = !released;
                return;
            case 0x36: /* right shift */
                keyboard_state.shift_right = !released;
                return;
            case 0x3A: /* caps lock */
                if (!released)
                    keyboard_state.caps_lock = !keyboard_state.caps_lock;
                return;
            case 0x0E: /* backspace */
                if (!released)
                {
                    putchar('\b');
                }
                return;
            default:
            {
                keyboard_state.keys[scancode] = !released;
                if (!released)
                {
                    const char ascii = translate_scancode(scancode);
                    keyboard_state.last_key = ascii;
                    enqueue_event(ascii, scancode);
                }
            }
        }
    }
}
