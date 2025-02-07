#include <oronyx/data/fonts.hpp>
#include <oronyx/drivers/graphics.hpp>

namespace onx
{
    namespace
    {
        limine_framebuffer *fb = nullptr;
        uint32_t *buffer = nullptr;
        size_t width = 0;
        size_t height = 0;
        size_t pitch = 0;
        size_t x_pos = 0;
        size_t y_pos = 0;
        uint32_t fg_color = 0xFFFFFFFF;
        uint32_t bg_color = 0x00000000;

        constexpr size_t BYTES_PER_GLYPH = FONT_HEIGHT;

        const uint8_t *get_glyph(uint32_t cp)
        {
            const size_t offset = cp * BYTES_PER_GLYPH;
            if (constexpr auto fonts_size = sizeof(FONTS);
                offset >= fonts_size)
                return &FONTS['?' * BYTES_PER_GLYPH];
            return &FONTS[offset];
        }

        bool is_wide_char(uint32_t cp)
        {
            return (cp >= 0x4E00 && cp <= 0x9FFF) || // cjk unified
                   (cp >= 0x3040 && cp <= 0x309F) || // hiragana
                   (cp >= 0x30A0 && cp <= 0x30FF) || // katakana
                   (cp >= 0xFF00 && cp <= 0xFFEF);   // fullwidth forms
        }

        size_t get_glyph_width(uint32_t cp)
        {
            return is_wide_char(cp) ? WIDE_WIDTH : BASIC_WIDTH;
        }

        size_t decode_utf8(const char *str, uint32_t *cp)
        {
            const auto first = static_cast<uint8_t>(*str);
            if ((first & 0x80) == 0)
            {
                *cp = first;
                return 1;
            }

            size_t len;
            if ((first & 0xE0) == 0xC0)
            {
                len = 2;
                *cp = first & 0x1F;
            }
            else if ((first & 0xF0) == 0xE0)
            {
                len = 3;
                *cp = first & 0x0F;
            }
            else if ((first & 0xF8) == 0xF0)
            {
                len = 4;
                *cp = first & 0x07;
            }
            else
            {
                *cp = 0xFFFD;
                return 1;
            }

            for (size_t i = 1; i < len; ++i)
            {
                const auto byte = static_cast<uint8_t>(str[i]);
                if ((byte & 0xC0) != 0x80)
                {
                    *cp = 0xFFFD;
                    return 1;
                }
                *cp = (*cp << 6) | (byte & 0x3F);
            }

            return len;
        }

        void _putchar(uint32_t cp)
        {
            const uint8_t *glyph = get_glyph(cp);
            size_t char_width = get_glyph_width(cp);

            if (x_pos * char_width >= width || y_pos * FONT_HEIGHT >= height)
            {
                return;
            }

            for (size_t y = 0; y < FONT_HEIGHT; ++y)
            {
                for (size_t x = 0; x < char_width; ++x)
                {
                    const size_t bit = 7 - (x % 8);
                    const size_t byte_offset = x / 8;
                    const size_t px = x_pos * char_width + x;
                    const size_t py = y_pos * FONT_HEIGHT + y;
                    if (px >= width || py >= height)
                        continue;

                    const size_t fb_offset = py * pitch + px;
                    const uint8_t current_byte = glyph[y + byte_offset * FONT_HEIGHT];
                    buffer[fb_offset] = (current_byte & (1 << bit)) ? fg_color : bg_color;
                }
            }

            x_pos++;
        }

        void scroll()
        {
            for (size_t y = FONT_HEIGHT; y < height; ++y)
            {
                for (size_t x = 0; x < width; ++x)
                    buffer[(y - FONT_HEIGHT) * pitch + x] = buffer[y * pitch + x];
            }

            for (size_t y = height - FONT_HEIGHT; y < height; ++y)
            {
                for (size_t x = 0; x < width; ++x)
                    buffer[y * pitch + x] = bg_color;
            }

            if (y_pos > 0)
                y_pos--;
        }

        void newline()
        {
            x_pos = 0;
            y_pos++;
            if (y_pos * FONT_HEIGHT >= height)
                scroll();
        }
    }

    void init(limine_framebuffer *framebuffer)
    {
        if (!framebuffer || !framebuffer->address)
            return;

        fb = framebuffer;
        buffer = static_cast<uint32_t *>(framebuffer->address);
        width = framebuffer->width;
        height = framebuffer->height;
        pitch = framebuffer->pitch / sizeof(uint32_t);
        x_pos = 0;
        y_pos = 0;
        fg_color = 0xFFFFFFFF;
        bg_color = 0x00000000;
        if (buffer && width && height && pitch)
            clear();
    }

    void clear()
    {
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
                buffer[y * pitch + x] = bg_color;
        }
        x_pos = 0;
        y_pos = 0;
    }

    void putchar(char c)
    {
        if (c == '\n')
        {
            newline();
        }
        else if (c == '\r')
        {
            x_pos = 0;
        }
        else
        {
            _putchar(c);
            if (x_pos * BASIC_WIDTH >= width)
                newline();
        }

        if (y_pos * FONT_HEIGHT >= height)
            scroll();
    }

    void write(const char *str)
    {
        while (*str)
        {
            if (*str == '\n')
            {
                newline();
                str++;
                continue;
            }

            uint32_t codepoint;
            str += decode_utf8(str, &codepoint);
            _putchar(codepoint);

            if (x_pos * BASIC_WIDTH >= width)
                newline();
        }
    }

    void write_line(const char *str)
    {
        write(str);
        newline();
    }

    void setcolor(uint32_t fg, uint32_t bg)
    {
        fg_color = fg;
        bg_color = bg;
    }

    void setcursor(size_t x, size_t y)
    {
        x_pos = x;
        y_pos = y;
    }

    size_t get_x()
    {
        return x_pos;
    }

    size_t get_y()
    {
        return y_pos;
    }

    size_t get_width()
    {
        return width;
    }

    size_t get_height()
    {
        return height;
    }
}
