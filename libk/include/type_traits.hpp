#pragma once

namespace onx
{
    template<typename T, T v>
    struct integral_constant
    {
        static constexpr T value = v;
        using value_type = T;
        using type = integral_constant<T, v>;

        constexpr explicit operator value_type() const noexcept
        {
            return value;
        }

        constexpr value_type operator()() const noexcept
        {
            return value;
        }
    };

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    template<typename T, typename U>
    struct is_same : false_type {};

    template<typename T>
    struct is_same<T, T> : true_type {};

    template<typename T>
    struct remove_cv
    {
        using type = T;
    };

    template<typename T>
    struct is_void : is_same<void, typename remove_cv<T>::type> {};

    template<typename T>
    struct remove_cv<const T>
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<volatile T>
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<const volatile T>
    {
        using type = T;
    };

    template<typename T>
    struct is_pointer_helper : false_type {};

    template<typename T>
    struct is_pointer_helper<T *> : true_type {};

    template<typename T>
    struct is_pointer : is_pointer_helper<typename remove_cv<T>::type> {};

    template<typename T>
    struct is_function : false_type {};

    template<typename Ret, typename... Args>
    struct is_function<Ret(Args...)> : true_type {};

    template<typename Ret, typename... Args>
    struct is_function<Ret(Args...) noexcept> : true_type {};

    template<bool Condition, typename TrueType, typename FalseType>
    struct conditional
    {
        using type = TrueType;
    };

    template<typename TrueType, typename FalseType>
    struct conditional<false, TrueType, FalseType>
    {
        using type = FalseType;
    };

    // Enable_if
    template<bool Condition, typename T = void>
    struct enable_if {};

    template<typename T>
    struct enable_if<true, T>
    {
        using type = T;
    };

    template<bool Condition, typename TrueFunc, typename FalseFunc>
    constexpr auto compile_time_select(TrueFunc true_func, FalseFunc false_func)
    {
        if constexpr (Condition)
        {
            return true_func();
        }
        else
        {
            return false_func();
        }
    }

    template<typename T>
    inline constexpr auto always_false = false;
}
