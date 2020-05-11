#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <array>
#include <utility>

namespace pure_simd {

    template <typename T, std::size_t N>
    using vector = std::array<T, N>;

    template <typename T, std::size_t N>
    using vector_n = vector<T, N>;

    namespace detail {

        template <typename Func, typename T, std::size_t N, std::size_t... Is>
        inline auto unroll(Func func, vector<T, N> x, vector<T, N> y, std::index_sequence<Is...>)
        {
            return std::array<T, N> { func(std::get<Is>(x), std::get<Is>(y))... };
        }

        template <typename Func, typename T, std::size_t N, std::size_t... Is>
        inline auto unroll(Func func, vector<T, N> x, std::index_sequence<Is...>)
        {
            return std::array<T, N> { func(std::get<Is>(x))... };
        }

    } // namespace detail

    template <typename Func, typename T, std::size_t N>
    inline auto unroll(Func func, vector<T, N> x, vector<T, N> y)
    {
        return detail::unroll(func, x, y, std::make_index_sequence<N> {});
    }

    template <typename Func, typename T, std::size_t N>
    inline auto unroll(Func func, vector<T, N> x)
    {
        return detail::unroll(func, x, std::make_index_sequence<N> {});
    }

    template <typename T, std::size_t N>
    inline auto operator+(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::plus<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline auto operator-(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::minus<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline auto operator-(vector<T, N> x)
    {
        return unroll(std::negate<T> {}, x);
    }

    template <typename T, std::size_t N>
    inline auto operator*(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::multiplies<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline auto operator/(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::divides<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline auto max(vector<T, N> x, vector<T, N> y)
    {
        return unroll([](auto a, auto b) { return std::max(a, b); }, x, y);
    }

    template <typename T, std::size_t N>
    inline auto min(vector<T, N> x, vector<T, N> y)
    {
        return unroll([](auto a, auto b) { return std::min(a, b); }, x, y);
    }

    template <typename To, typename From, std::size_t N>
    inline auto cast(vector<From, N> x)
    {
        return unroll([](auto a) { return static_cast<To>(a); }, x);
    }

    namespace detail {

        template <typename... Ts>
        void eval_args(Ts const&...) {}

        template <typename T, std::size_t N, std::size_t... Is>
        inline void store(vector<T, N> x,
            T* array,
            std::index_sequence<Is...>)
        {
            eval_args((array[Is] = x[Is])...);
        }

    } // namespace detail

    template <typename T, std::size_t N>
    inline void store(vector<T, N> x, T* array)
    {
        detail::store(x, array, std::make_index_sequence<N> {});
    }

    namespace detail {

        template <typename T>
        constexpr T identity(std::size_t, T x) { return x; }

        template <typename>
        struct scalar_impl;

        template <typename T, std::size_t N>
        constexpr std::size_t size_impl(vector<T, N>*) { return N; }

        template <typename V>
        constexpr std::size_t size()
        {
            V* p = nullptr;
            return size_impl(p);
        }

        template <typename T, std::size_t N>
        struct scalar_impl<vector<T, N>> {
            template <std::size_t... Is>
            inline auto operator()(T x, std::index_sequence<Is...>)
            {
                return std::array<T, N> { identity(Is, x)... };
            }
        };

    } // namespace detail

    template <typename V>
    inline auto scalar(typename V::value_type x)
    {
        return detail::scalar_impl<V> {}(x, std::make_index_sequence<detail::size<V>()>());
    }

    namespace detail {

        template <typename, typename>
        struct ascend_from_impl;

        template <typename T, std::size_t N, std::size_t... Is>
        struct ascend_from_impl<vector<T, N>, std::index_sequence<Is...>> {
            inline auto operator()(T start, T step)
            {
                return std::array<T, N> { identity<T>(Is, start + step * Is)... };
            }
        };

    } // namespace detail

    template <typename V>
    inline auto ascend_from(std::tuple_element_t<0, V> start, std::tuple_element_t<0, V> step)
    {
        return detail::ascend_from_impl<V, std::make_index_sequence<std::tuple_size<V>::value>> {}(start, step);
    }

} // namespace pure_simd

#endif /* PURE_SIMD_H */
