#pragma once

#ifndef __ORNYX__ATOMIC__
#define __ORNYX__ATOMIC__

namespace onx
{
    template<typename T>
    class atomic
    {
    public:
        atomic() : _value(0) {}

        atomic(T init) : _value(init) {}

        // TODO: add aarch64 & more arch support
        // TODO: weak & strong
        bool compare_exchange(T& expected, T desired) noexcept
        {
            #if defined(__x86_64__)
                bool result;
                asm volatile(
                    "lock cmpxchg %2, %1\n\t"
                    "sete %0"
                    : "=q"(result), "+m"(_value), "+a"(expected)
                    : "r"(desired)
                    : "cc", "memory"
                );
                return result;
            #endif
            return true; // for compat.
        }

        T load() const noexcept
        {
            return __atomic_load_n(&_value, __ATOMIC_SEQ_CST);
        }

        void store(T new_value) noexcept
        {
            __atomic_store_n(&_value, new_value, __ATOMIC_SEQ_CST);
        }

        T fetch_add(T arg) noexcept
        {
        #if defined(__x86_64__)
            asm volatile("lock xadd %0, %1"
                         : "+r"(arg), "+m"(_value)
                         :
                         : "memory");
            return arg;
        #else
            return __atomic_fetch_add(&_value, arg, __ATOMIC_SEQ_CST);
        #endif
        }


    private:
        volatile T _value;
    };
}

#endif