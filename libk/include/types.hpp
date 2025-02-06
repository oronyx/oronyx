#pragma once

#ifndef __ORNYX__TYPES__
#define __ORNYX__TYPES__

#include <stdint.h>
#include <stddef.h>

using bool_t = bool;

#ifndef size_t
using size_t = __SIZE_TYPE__;
#endif

struct x86_64;
struct aarch64;

/* TODO: support more architecture later */
#if defined(__x86_64__)
using current_arch = x86_64;
#elif defined(__aarch64__)
using current_arch = aarch64;
#else
#error "Unsupported architecture"
#endif

constexpr auto null = nullptr;

#endif