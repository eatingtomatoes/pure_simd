#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <array>
#include <functional>
#include <tuple>

namespace pure_simd {

    namespace trait {

        // Every concrete vector type should specialize the `as_vector' struct.
        template <typename V>
        struct as_vector {
            // The vector type after original elements' types replaced with new ones
            // template <typename... Args>
            // using with_element_t;

            static constexpr bool is_vector = false;

            // static constexpr size;

            // template <std::size_t I, typename V>
            // constexpr auto elem_at(V x);
        };

        template <typename V, typename... Args>
        using with_element_t = typename as_vector<V>::template with_element<Args...>;

        template <typename T>
        constexpr bool is_vector_v = as_vector<T>::is_vector;

        template <typename T>
        using must_be_vector = std::enable_if_t<is_vector_v<T>>;

        template <typename V>
        constexpr std::size_t size_v = as_vector<V>::size;

        template <std::size_t I, typename V>
        constexpr auto elem_at(V x)
        {
            return as_vector<V>::template elem_at<I>(x);
        }

        template <typename T>
        using index_sequence_of = std::make_index_sequence<size_v<T>>;

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

            static constexpr std::size_t size = sizeof...(Args);

            template <typename I>
            static constexpr auto elem_at(tuple<Args...> x)
            {
                return std::get<I>(x);
            }
        };

    }

    namespace detail {

        template <typename T, std::size_t>
        using type_of = T;

        template <typename, typename>
        struct tuple_n_impl {
        };

        template <typename T, std::size_t... Is>
        struct tuple_n_impl<T, std::index_sequence<Is...>> {
            using type = tuple<type_of<T, Is>...>;
        };

    } // namespace detail

    template <typename T, std::size_t N>
    using tuple_n = typename detail::tuple_n_impl<T, std::make_index_sequence<N>>::type;

    // `array' is another vector type.
    template <typename T, std::size_t N1, std::size_t Alignment = 32>
    struct alignas(Alignment) array {
        using value_type = T;

        using size_type = std::size_t;

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

        T data[N];
    };

    namespace trait {

        template <typename T, std::size_t N, std::size_t Alignment>
        struct as_vector<array<T, N, Alignment>> {
            template <typename U, typename...>
            using with_element = array<U, N, Alignment>;

            static constexpr bool is_vector = true;

            static constexpr std::size_t size = N;

            template <std::size_t I>
            static constexpr auto elem_at(array<T, N, Alignment> x)
            {
                return x[I];
            }
        };

    } // namespace trait

    namespace detail {

        template <typename F, typename V, std::size_t... Is>
        inline auto unroll_impl(F func, V x, std::index_sequence<Is...>)
            -> with_element_t<V, decltype(func(elem_at<Is>(x)))...>
        {
            return { func(elem_at<Is>(x))... };
        }

        template <typename F, typename V, std::size_t... Is>
        inline auto unroll_impl(F func, V x, V y, std::index_sequence<Is...>)
            -> with_element_t<V, decltype(func(elem_at<Is>(x), elem_at<Is>(y)))...>
        {
            return { func(elem_at<Is>(x), elem_at<Is>(y))... };
        }

    } // namespace detail

    template <typename F, typename V, typename = must_be_vector<V>>
    inline auto unroll(F func, V x)
    {
        return detail::unroll_impl(func, x, index_sequence_of<V> {});
    }

    template <typename F, typename V, typename = must_be_vector<V>>
    inline auto unroll(F func, V x, V y)
    {
        return detail::unroll_impl(func, x, y, index_sequence_of<V> {});
    }

    template <typename F, typename V, typename = must_be_vector<V>>
    inline auto unroll(V x, F func)
    {
        return detail::unroll_impl(func, x, index_sequence_of<V> {});
    }

    template <typename F, typename V, typename = must_be_vector<V>>
    inline auto unroll(V x, V y, F func)
    {
        return detail::unroll_impl(func, x, y, index_sequence_of<V> {});
    }

    template <typename V, typename = must_be_vector<V>>
    inline V operator+(V x, V y)
    {
        return unroll([](auto a, auto b) { return a + b; }, x, y);
    }

    template <typename V, typename = must_be_vector<V>>
    inline V operator-(V x, V y)
    {
        return unroll([](auto a, auto b) { return a - b; }, x, y);
    }

    template <typename V, typename = must_be_vector<V>>
    inline V operator-(V x)
    {
        return unroll([](auto a) { return -a; }, x);
    }

    template <typename V, typename = must_be_vector<V>>
    inline V operator*(V x, V y)
    {
        return unroll([](auto a, auto b) { return a * b; }, x, y);
    }

    template <typename V, typename = must_be_vector<V>>
    inline V operator/(V x, V y)
    {
        return unroll([](auto a, auto b) { return a / b; }, x, y);
    }

    template <typename V, typename = must_be_vector<V>>
    inline V max(V x, V y)
    {
        return unroll([](auto a, auto b) { return std::max(a, b); }, x, y);
    }

    template <typename V, typename = must_be_vector<V>>
    inline V min(V x, V y)
    {
        return unroll([](auto a, auto b) { return std::min(a, b); }, x, y);
    }

    template <typename T, typename V, typename = must_be_vector<V>>
    inline with_element_t<V, T> cast(V x)
    {
        return unroll([](auto a) { return static_cast<T>(a); }, x);
    }

    namespace detail {

        template <typename V, typename T, std::size_t... Is>
        inline void store_to_impl(V x, T* dst, std::index_sequence<Is...>)
        {
            [](auto...) {}(((dst[Is] = elem_at<Is>(x)), true)...);
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    inline void store_to(V x, T* dst)
    {
        detail::store_to_impl(x, dst, index_sequence_of<V> {});
    }

    namespace detail {

        template <std::size_t, typename T>
        constexpr T identity(T x) { return x; }

        template <typename V, typename T, std::size_t... Is>
        inline V scalar_impl(T x, std::index_sequence<Is...>)
        {
            return { identity<Is>(x)... };
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    inline V scalar(T x)
    {
        return detail::scalar_impl<V>(x, index_sequence_of<V> {});
    }

    namespace detail {

        template <typename V, typename T, std::size_t... Is>
        inline V load_from_impl(const T* src, std::index_sequence<Is...>)
        {
            return { src[Is]... };
        }

    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    inline V load_from(const T* src)
    {
        return detail::load_from_impl<V>(src, index_sequence_of<V> {});
    }

    namespace detail {

        template <typename V, typename T, typename S, typename I, I... Is>
        inline V ascend_from_impl(T start, S step, std::integer_sequence<I, Is...>)
        {
            return { (start + step * Is)... };
        };

    } // namespace detail

    template <typename V, typename I = std::size_t, typename T, typename S, typename = must_be_vector<V>>
    inline V ascend_from(T start, S step)
    {
        return detail::ascend_from_impl<V>(
            start, step, std::make_integer_sequence<I, size_v<V>> {} //
        );
    }

    namespace detail {

        constexpr bool is_power_of_two(std::size_t N)
        {
            return (N & (N - 1)) == 0;
        }

    } // namespace detail

    template <typename T, T Value>
    struct constexpr_value_t {
        using type = T;
        static constexpr T value = Value;
    };

    template <std::size_t N>
    using constexpr_size_t = constexpr_value_t<std::size_t, N>;

    namespace detail {

        template <typename S, S Stride, bool Zero>
        struct unroll_loop_impl;

        template <typename S, S Value>
        struct unroll_loop_impl<S, Value, true> {
            template <typename... Args>
            auto operator()(Args...) {}
        };

        template <typename S, S Stride>
        struct unroll_loop_impl<S, Stride, false> {
            template <typename I, typename F>
            auto operator()(I start, S iterations, F func)
                -> decltype(func(constexpr_size_t<Stride> {}, start), void())
            {
                static_assert(Stride > 0ull && detail::is_power_of_two(Stride), "");

                auto rem = iterations % Stride;
                auto bound = start + (iterations - rem);

                for (auto i = start; i < bound; i += Stride) {
                    func(constexpr_value_t<S, Stride> {}, i);
                }

                if (rem > 0) {
                    unroll_loop_impl<S, Stride / 2, Stride / 2 == S {}> {}(bound, rem, func);
                }
            }
        };

    } // namespace detail

    template <typename S, S Stride, typename I, typename F>
    inline auto unroll_loop(I start, S iterations, F func)
        -> decltype(func(constexpr_value_t<S, Stride> {}, start), void())
    {
        detail::unroll_loop_impl<S, Stride, Stride == S {}> {}(start, iterations, func);
    }

    template <std::size_t Stride, typename I, typename F>
    inline auto unroll_loop(I start, std::size_t iterations, F func)
        -> decltype(func(constexpr_size_t<Stride> {}, start), void())
    {
        detail::unroll_loop_impl<std::size_t, Stride, Stride == 0ull> {}(start, iterations, func);
    }

} // namespace pure_simd

#endif /* PURE_SIMD_H */
