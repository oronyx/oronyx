#include <ctypes.hpp>

namespace onx
{
    bool isalpha(int c) noexcept 
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    bool isdigit(int c) noexcept 
    {
        return (c >= '0' && c <= '9');
    }

    bool isalnum(int c) noexcept 
    {
        return isalpha(c) || isdigit(c);
    }

    bool isspace(int c) noexcept
    {
        return (c == ' ' || c == '\t' || c == '\n' || 
                c == '\r' || c == '\f' || c == '\v');
    }

    bool ispunct(int c) noexcept 
    {
        return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) || 
               (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
    }

    bool isprint(int c) noexcept 
    {
        return (c >= 32 && c <= 126);
    }

    bool isupper(int c) noexcept 
    {
        return (c >= 'A' && c <= 'Z');
    }

    bool islower(int c) noexcept 
    {
        return (c >= 'a' && c <= 'z');
    }

    bool isxdigit(int c) noexcept 
    {
        return isdigit(c) || 
               (c >= 'A' && c <= 'F') || 
               (c >= 'a' && c <= 'f');
    }

    int toupper(int c) noexcept 
    {
        return (islower(c)) ? (c - 'a' + 'A') : c;
    }

    int tolower(int c) noexcept 
    {
        return (isupper(c)) ? (c - 'A' + 'a') : c;
    }
}