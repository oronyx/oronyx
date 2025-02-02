#pragma once

#ifndef __ORNYX__CTYPE__
#define __ORNYX__CTYPE__

#include <types.hpp>

namespace onx
{
    bool isalpha(int c) noexcept;
    bool isdigit(int c) noexcept;
    bool isalnum(int c) noexcept;
    bool isspace(int c) noexcept;
    bool ispunct(int c) noexcept;
    bool isprint(int c) noexcept;
    bool isupper(int c) noexcept;
    bool islower(int c) noexcept;
    bool isxdigit(int c) noexcept;

    int toupper(int c) noexcept;
    int tolower(int c) noexcept;
}

#endif