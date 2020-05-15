#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <array>
#include <functional>
#include <tuple>

namespace pure_simd {

    using size_t = std::size_t;

    namespace trait {

        // Every concrete vector type should specialize the `as_vector' struct.
        template <typename V>
        struct as_vector {
            // The vector type after original elements' types replaced with new ones
            // template <typename... Args>
            // using with_element_t;

            static constexpr bool is_vector = false;

            // static constexpr size;

            // template <size_t I, typename V>
            // auto elem_at(V x);
        };

        template <typename V, typename... Args>
        using with_element_t = typename as_vector<V>::template with_element<Args...>;

        template <typename T>
        constexpr bool is_vector_v = as_vector<T>::is_vector;

        template <typename T>
        using must_be_vector = std::enable_if_t<is_vector_v<T>>;

        template <typename V>
        constexpr size_t size_v = as_vector<V>::size;

        template <size_t I, typename V>
        constexpr auto elem_at(V x)
        {
            return as_vector<V>::template elem_at<I>(x);
        }

        template <typename T>
        using index_sequence_of = std::make_index_sequence<size_v<T>>;

        template <typename V1, typename V2>
        struct is_compatible : std::false_type {
        };

        template <typename V1, typename V2>
        using must_be_compatible = std::enable_if_t<is_compatible<V1, V2>::value>;

    } // namespace trait

    using namespace trait;

    template <typename... Args>
    using tuple = std::tuple<Args...>;

    namespace trait {

        template <typename... Args>
        struct as_vector<tuple<Args...>> {
            template <typename... Args1>
            using with_element = tuple<Args1...>;

            static constexpr bool is_vector = true;

            static constexpr size_t size = sizeof...(Args);

            template <size_t I>
            static constexpr auto elem_at(tuple<Args...> x)
            {
                return std::get<I>(x);
            }
        };

        template <typename... Args0, typename... Args1>
        struct is_compatible<tuple<Args0...>, tuple<Args1...>>
            : std::integral_constant<bool, sizeof...(Args0) == sizeof...(Args1)> {
        };

        template <typename>
        struct is_tuple : std::false_type {
        };

        template <typename... Args>
        struct is_tuple<tuple<Args...>> {
        };

        template <typename V>
        using not_tuple = std::enable_if_t<!is_tuple<V>::value>;
    }

    namespace detail {

        template <typename T, size_t>
        using type_of = T;

        template <typename, typename>
        struct tuple_n_impl {
        };

        template <typename T, size_t... Is>
        struct tuple_n_impl<T, std::index_sequence<Is...>> {
            using type = tuple<type_of<T, Is>...>;
        };

    } // namespace detail

    template <typename T, size_t N>
    using tuple_n = typename detail::tuple_n_impl<T, std::make_index_sequence<N>>::type;

    // `array' is another vector type.
    template <typename T, size_t N1, size_t Align = 32>
    struct alignas(Align) array {
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

        static constexpr size_t N = N1;

        constexpr T& operator[](size_t pos) { return data[pos]; }

        constexpr T operator[](size_t pos) const { return data[pos]; }

        constexpr iterator begin() { return data; }

        constexpr iterator end() { return data + N; }

        constexpr const_iterator begin() const { return data; }

        constexpr const_iterator end() const { return data + N; }

        constexpr const_iterator cbegin() const { return data; }

        constexpr const_iterator cend() const { return data + N; }

        T data[N];
    };

    namespace trait {

        template <typename T, size_t N, size_t Align>
        struct as_vector<array<T, N, Align>> {
            template <typename U, typename...>
            using with_element = array<U, N, Align>;

            static constexpr bool is_vector = true;

            static constexpr size_t size = N;

            template <size_t I>
            static constexpr auto elem_at(array<T, N, Align> x)
            {
                return x[I];
            }
        };

        template <typename T0, size_t N0, size_t Align0,
            typename T1, size_t N1, size_t Align1>
        struct is_compatible<array<T0, N0, Align0>, array<T1, N1, Align1>>
            : std::integral_constant<bool, N0 == N1> {
        };

    } // namespace trait

    namespace detail {

        template <size_t Align, typename T, typename... Args, size_t... Is>
        constexpr auto to_array(tuple<T, Args...> xs, std::index_sequence<Is...>)
        {
            return pure_simd::array<T, sizeof...(Is), Align> { std::get<Is>(xs)... };
        }

        template <typename T, size_t N, size_t Align, size_t... Is>
        constexpr auto to_tuple(array<T, N, Align> xs, std::index_sequence<Is...>)
        {
            return tuple_n<T, N> { xs[Is]... };
        }

    } // namespace detail

    template <size_t Align = 32, typename... Args>
    constexpr auto to_array(tuple<Args...> xs)
    {
        return detail::to_array<Align>(xs, std::index_sequence_for<Args...> {});
    }

    template <typename T, size_t N, size_t Align>
    constexpr auto to_tuple(array<T, N, Align> xs)
    {
        return detail::to_tuple(xs, std::make_index_sequence<N> {});
    }

    namespace detail {
        template <typename F, typename V, size_t... Is>
        constexpr auto unroll_impl(F func, V x, std::index_sequence<Is...>)
            -> with_element_t<V, decltype(func(elem_at<Is>(x)))...>
        {
            return { func(elem_at<Is>(x))... };
        }

        template <typename F, typename V0, typename V1, size_t... Is>
        constexpr auto unroll_impl(F func, V0 x, V1 y, std::index_sequence<Is...>)
            -> with_element_t<V0, decltype(func(elem_at<Is>(x), elem_at<Is>(y)))...>
        {
            return { func(elem_at<Is>(x), elem_at<Is>(y))... };
        }

    } // namespace detail

    template <typename F, typename V, typename = must_be_vector<V>>
    constexpr auto unroll(F func, V x)
    {
        return detail::unroll_impl(func, x, index_sequence_of<V> {});
    }

    template <
        typename F, typename V0, typename V1,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = must_be_compatible<V0, V1>>
    constexpr auto unroll(F func, V0 x, V1 y)
    {
        return detail::unroll_impl(func, x, y, index_sequence_of<V0> {});
    }

    template <typename F, typename V, typename = must_be_vector<V>>
    constexpr auto unroll(V x, F func)
    {
        return detail::unroll_impl(func, x, index_sequence_of<V> {});
    }

    template <
        typename F, typename V0, typename V1,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = must_be_compatible<V0, V1>>
    constexpr auto unroll(V0 x, V1 y, F func)
    {
        return detail::unroll_impl(func, x, y, index_sequence_of<V0> {});
    }

#define OVERLOAD_BINARY_OPERATOR(op)                                \
    template <                                                      \
        typename V0, typename V1,                                   \
        typename = must_be_vector<V0>,                              \
        typename = must_be_vector<V1>,                              \
        typename = must_be_compatible<V0, V1>>                      \
    constexpr auto operator op(V0 x, V1 y)                             \
    {                                                               \
        return unroll(x, y, [](auto a, auto b) { return a op b; }); \
    }

#define OVERLOAD_UNARY_OPERATOR(op)                     \
    template <typename V, typename = must_be_vector<V>> \
    constexpr auto operator op(V x)                        \
    {                                                   \
        return unroll(x, [](auto a) { return op a; });  \
    }

#define OVERLOAD_COMPARISON_OPERATOR(op)                                           \
    template <                                                                     \
        typename V0,                                                               \
        typename V1,                                                               \
        typename = must_be_vector<V0>,                                             \
        typename = must_be_vector<V1>,                                             \
        typename = std::enable_if_t<!is_tuple<V0>::value || !is_tuple<V1>::value>> \
    constexpr auto operator op(V0 x, V1 y)                                            \
    {                                                                              \
        return unroll(x, y, [](auto a, auto b) { return a op b; });                \
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
    constexpr V max(V x, V y)
    {
        return unroll(x, y, [](auto a, auto b) { return std::max(a, b); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr V min(V x, V y)
    {
        return unroll(x, y, [](auto a, auto b) { return std::min(a, b); });
    }

    template <typename T, typename V, typename = must_be_vector<V>>
    constexpr auto cast_to(V x)
    {
        return unroll(x, [](auto a) { return static_cast<T>(a); });
    }

    namespace detail {

        template <typename V, typename T, size_t... Is>
        constexpr void store_to_impl(V x, T* dst, std::index_sequence<Is...>)
        {
            [](auto...) {}(((dst[Is] = elem_at<Is>(x)), true)...);
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr void store_to(V x, T* dst)
    {
        detail::store_to_impl(x, dst, index_sequence_of<V> {});
    }

    namespace detail {

        template <size_t, typename T>
        constexpr T identity(T x) { return x; }

        template <typename V, typename T, size_t... Is>
        constexpr V scalar_impl(T x, std::index_sequence<Is...>)
        {
            return { identity<Is>(x)... };
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr V scalar(T x)
    {
        return detail::scalar_impl<V>(x, index_sequence_of<V> {});
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

    template <typename V, typename I = size_t, typename T, typename S, typename = must_be_vector<V>>
    constexpr V iota(T start, S step)
    {
        return detail::iota_impl<V>(
            start, step, std::make_integer_sequence<I, size_v<V>> {} //
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

} // namespace pure_simd

#endif /* PURE_SIMD_H */
