#pragma once

#ifndef __ORNYX__TYPES__
#define __ORNYX__TYPES__

using bool_t = bool;

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;

using int8_t = signed char;
using int16_t = signed short;
using int32_t = signed int;
using int64_t = signed long long;

using intptr_t = signed long long;
using uintptr_t = unsigned long long;

using size_t = unsigned long long;

constexpr decltype(nullptr) null = nullptr;

#endif