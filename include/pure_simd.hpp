#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <climits>
#include <functional>

namespace pure_simd {

    using size_t = std::size_t;

    template <typename T, size_t N, size_t Align = 32>
    struct alignas(Align) vector {
        template <typename U>
        using with_value_t = vector<U, N, Align>;

        using value_type = T;

        using size_type = size_t;

        using difference_type = std::ptrdiff_t;

        using reference = value_type&;

        using const_reference = const value_type&;

        using pointer = value_type*;

        using const_pointer = const value_type*;

        using iterator = pointer;

        using const_iterator = const_pointer;

        using reverse_iterator = std::reverse_iterator<iterator>;

        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr T& operator[](size_t pos) { return data[pos]; }

        constexpr T operator[](size_t pos) const { return data[pos]; }

        constexpr iterator begin() { return data; }

        constexpr iterator end() { return data + N; }

        constexpr const_iterator begin() const { return data; }

        constexpr const_iterator end() const { return data + N; }

        constexpr const_iterator cbegin() const { return data; }

        constexpr const_iterator cend() const { return data + N; }

        static constexpr size_t size() { return N; }

        T data[N];
    };

    inline namespace trait {

        template <typename T>
        struct is_vector : std::false_type {
        };

        template <typename T, size_t N, size_t A>
        struct is_vector<vector<T, N, A>> : std::true_type {
        };

        template <typename T>
        using must_be_vector = std::enable_if_t<is_vector<T>::value>;

        template <typename T>
        using index_sequence_of = std::make_index_sequence<T::size()>;

        template <typename V1, typename V2>
        struct same_size : std::false_type {
        };

        template <
            typename T0, size_t N0, size_t A0,
            typename T1, size_t N1, size_t A1>
        struct same_size<vector<T0, N0, A0>, vector<T1, N1, A1>>
            : std::integral_constant<bool, N0 == N1> {
        };

        template <typename V1, typename V2>
        using assert_same_size = std::enable_if_t<same_size<V1, V2>::value>;

    } // namespace trait

    namespace detail {
        template <typename F, typename V, size_t... Is>
        constexpr auto unroll_impl(F func, V xs, std::index_sequence<Is...>)
            -> typename V::template with_value_t<decltype(func(xs[0]))>
        {
            return { func(xs[Is])... };
        }

        template <typename F, typename V0, typename V1, size_t... Is>
        constexpr auto unroll_impl(F func, V0 xs, V1 ys, std::index_sequence<Is...>)
            -> typename V0::template with_value_t<decltype(func(xs[0], ys[0]))>
        {
            return { func(xs[Is], ys[Is])... };
        }

    } // namespace detail

    template <typename F, typename V, typename = must_be_vector<V>>
    constexpr auto unroll(F func, V xs)
    {
        return detail::unroll_impl(func, xs, index_sequence_of<V> {});
    }

    template <
        typename F, typename V0, typename V1,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = assert_same_size<V0, V1>>
    constexpr auto unroll(F func, V0 xs, V1 ys)
    {
        return detail::unroll_impl(func, xs, ys, index_sequence_of<V0> {});
    }

    template <typename F, typename V, typename = must_be_vector<V>>
    constexpr auto unroll(V xs, F func)
    {
        return detail::unroll_impl(func, xs, index_sequence_of<V> {});
    }

    template <
        typename F, typename V0, typename V1,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = assert_same_size<V0, V1>>
    constexpr auto unroll(V0 xs, V1 ys, F func)
    {
        return detail::unroll_impl(func, xs, ys, index_sequence_of<V0> {});
    }

#define OVERLOAD_BINARY_OPERATOR(op)                                  \
    template <                                                        \
        typename V0, typename V1,                                     \
        typename = must_be_vector<V0>,                                \
        typename = must_be_vector<V1>,                                \
        typename = assert_same_size<V0, V1>>                          \
    constexpr auto operator op(V0 xs, V1 ys)                          \
    {                                                                 \
        return unroll(xs, ys, [](auto a, auto b) { return a op b; }); \
    }

#define OVERLOAD_UNARY_OPERATOR(op)                     \
    template <typename V, typename = must_be_vector<V>> \
    constexpr auto operator op(V xs)                    \
    {                                                   \
        return unroll(xs, [](auto a) { return op a; }); \
    }

#define OVERLOAD_COMPARISON_OPERATOR(op)                              \
    template <                                                        \
        typename V0,                                                  \
        typename V1,                                                  \
        typename = must_be_vector<V0>,                                \
        typename = must_be_vector<V1>,                                \
        typename = assert_same_size<V0, V1>>                          \
    constexpr auto operator op(V0 xs, V1 ys)                          \
    {                                                                 \
        return unroll(xs, ys, [](auto a, auto b) { return a op b; }); \
    }

    OVERLOAD_BINARY_OPERATOR(+)

    OVERLOAD_BINARY_OPERATOR(-)

    OVERLOAD_UNARY_OPERATOR(-)

    OVERLOAD_BINARY_OPERATOR(*)

    OVERLOAD_BINARY_OPERATOR(/)

    OVERLOAD_BINARY_OPERATOR(%)

    OVERLOAD_BINARY_OPERATOR(^)

    OVERLOAD_BINARY_OPERATOR(&)

    OVERLOAD_BINARY_OPERATOR(|)

    OVERLOAD_UNARY_OPERATOR(~)

    OVERLOAD_UNARY_OPERATOR(!)

    OVERLOAD_COMPARISON_OPERATOR(<)

    OVERLOAD_COMPARISON_OPERATOR(>)

    OVERLOAD_BINARY_OPERATOR(<<)

    OVERLOAD_BINARY_OPERATOR(>>)

    OVERLOAD_COMPARISON_OPERATOR(==)

    OVERLOAD_COMPARISON_OPERATOR(!=)

    OVERLOAD_COMPARISON_OPERATOR(<=)

    OVERLOAD_COMPARISON_OPERATOR(>=)

    OVERLOAD_BINARY_OPERATOR(&&)

    OVERLOAD_BINARY_OPERATOR(||)

#undef OVERLOAD_BINARY_OPERATOR
#undef OVERLOAD_UNARY_OPERATOR
#undef OVERLOAD_COMPARISON_OPERATOR

    template <typename V, typename = must_be_vector<V>>
    constexpr V max(V xs, V ys)
    {
        return unroll(xs, ys, [](auto a, auto b) { return std::max(a, b); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr V min(V xs, V ys)
    {
        return unroll(xs, ys, [](auto a, auto b) { return std::min(a, b); });
    }

    template <typename T, typename V, typename = must_be_vector<V>>
    constexpr auto cast_to(V xs)
    {
        return unroll(xs, [](auto a) { return static_cast<T>(a); });
    }

    namespace detail {

        template <typename V, typename T, size_t... Is>
        constexpr void store_to_impl(V xs, T* dst, std::index_sequence<Is...>)
        {
            [](auto...) {}(((dst[Is] = xs[Is]), true)...);
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr void store_to(V xs, T* dst)
    {
        detail::store_to_impl(xs, dst, index_sequence_of<V> {});
    }

    namespace detail {

        template <size_t, typename T>
        constexpr T identity(T xs) { return xs; }

        template <typename V, typename T, size_t... Is>
        constexpr V scalar_impl(T xs, std::index_sequence<Is...>)
        {
            return { identity<Is>(xs)... };
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr V scalar(T xs)
    {
        return detail::scalar_impl<V>(xs, index_sequence_of<V> {});
    }

    namespace detail {

        template <typename V, typename T, size_t... Is>
        constexpr V load_from_impl(const T* src, std::index_sequence<Is...>)
        {
            return { src[Is]... };
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr V load_from(const T* src)
    {
        return detail::load_from_impl<V>(src, index_sequence_of<V> {});
    }

    namespace detail {

        template <typename V, typename T, typename S, typename I, I... Is>
        constexpr V iota_impl(T start, S step, std::integer_sequence<I, Is...>)
        {
            return { (start + step * Is)... };
        };

    } // namespace detail

    template <
        typename V, typename I = size_t,
        typename T, typename S,
        typename = must_be_vector<V>>
    constexpr V iota(T start, S step)
    {
        return detail::iota_impl<V>(
            start, step, std::make_integer_sequence<I, V::size()> {} //
        );
    }

    namespace detail {

        constexpr bool is_power_of_two(size_t N)
        {
            return (N & (N - 1)) == 0;
        }

    } // namespace detail

    template <size_t N>
    using size_constant = std::integral_constant<size_t, N>;

    namespace detail {

        template <typename S, S Step, bool ZeroStep>
        struct unroll_loop_impl;

        template <typename S, S Step>
        struct unroll_loop_impl<S, Step, true> {
            template <typename... Args>
            auto operator()(Args...) {}
        };

        template <typename S, S Step>
        struct unroll_loop_impl<S, Step, false> {
            template <typename I, typename F>
            auto operator()(I start, S iterations, F func)
                -> decltype(func(std::integral_constant<S, Step> {}, start), void())
            {
                static_assert(Step > 0ull && detail::is_power_of_two(Step), "");

                auto rem = iterations % Step;
                auto bound = start + (iterations - rem);

                for (auto i = start; i < bound; i += Step) {
                    func(std::integral_constant<S, Step> {}, i);
                }

                if (rem > 0) {
                    unroll_loop_impl<S, Step / 2, Step / 2 == S {}> {}(bound, rem, func);
                }
            }
        };

    } // namespace detail

    template <typename S, S MaxStep, typename I, typename F>
    constexpr auto unroll_loop(I start, S iterations, F func)
        -> decltype(func(std::integral_constant<S, MaxStep> {}, start), void())
    {
        detail::unroll_loop_impl<S, MaxStep, MaxStep == S {}> {}(start, iterations, func);
    }

    template <size_t MaxStep, typename I, typename F>
    constexpr auto unroll_loop(I start, size_t iterations, F func)
        -> decltype(func(size_constant<MaxStep> {}, start), void())
    {
        detail::unroll_loop_impl<size_t, MaxStep, MaxStep == 0ull> {}(start, iterations, func);
    }

    namespace detail {
        template <typename V, typename T, size_t... Is>
        constexpr V scatter_bits_impl(T bits, std::index_sequence<Is...>)
        {
            return { static_cast<typename V::value_type>(((bits >> Is) & 0x01))... };
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr V scatter_bits(T bits)
    {
        return detail::scatter_bits_impl<V>(bits, index_sequence_of<V> {});
    }

    namespace detail {
        template <typename T, size_t Idx, typename V>
        constexpr T gather_bit(V x) { return static_cast<T>((x[Idx] != 0) << Idx); }

        template <typename T, typename V, size_t... Is>
        constexpr T gather_bits_impl(V xs, std::index_sequence<Is...>)
        {
            return (gather_bit<T, Is>(xs) | ...);
        }
    } // namespace detail

    template <typename T, typename V, typename = must_be_vector<V>>
    constexpr T gather_bits(V xs)
    {
        static_assert((sizeof(T) * CHAR_BIT) >= xs.size());
        return detail::gather_bits_impl<T>(xs, index_sequence_of<V> {});
    }

} // namespace pure_simd

#endif /* PURE_SIMD_H */
