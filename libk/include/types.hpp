#pragma once

#ifndef __ORNYX__TYPES__
#define __ORNYX__TYPES__

#include <stdint.h>
#include <stddef.h>

using bool_t = bool;

#ifndef size_t
using size_t = __SIZE_TYPE__;
#endif

constexpr decltype(nullptr) null = nullptr;

#endif