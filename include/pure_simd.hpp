#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <array>
#include <functional>

namespace pure_simd {

    namespace detail {
    
        template <typename Func, typename T, std::size_t N, std::size_t... Is>
        inline auto unroll(Func func, std::array<T, N> x, std::array<T, N> y, std::index_sequence<Is...>)
          -> std::array<decltype(func(x[0], y[0])), N>
        {
            return { func(x[Is], y[Is])... };
        }

        template <typename Func, typename T, std::size_t N, std::size_t... Is>
        inline auto unroll(Func func, std::array<T, N> x, std::index_sequence<Is...>)
          -> std::array<decltype(func(x[0])), N>          
        {
            return { func(x[Is])... };
        }

    } // namespace detail

    template <typename Func, typename T, std::size_t N>
    inline auto unroll(Func func, std::array<T, N> x, std::array<T, N> y)
    {
        return detail::unroll(func, x, y, std::make_index_sequence<N> {});
    }

    template <typename Func, typename T, std::size_t N>
    inline auto unroll(Func func, std::array<T, N> x)
    {
        return detail::unroll(func, x, std::make_index_sequence<N> {});
    }

    template <typename T, std::size_t N>
    inline std::array<T, N> operator+(std::array<T, N> x, std::array<T, N> y)
    {
        return unroll(std::plus<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline std::array<T, N> operator-(std::array<T, N> x, std::array<T, N> y)
    {
        return unroll(std::minus<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline std::array<T, N> operator-(std::array<T, N> x)
    {
        return unroll(std::negate<T> {}, x);
    }

    template <typename T, std::size_t N>
    inline std::array<T, N> operator*(std::array<T, N> x, std::array<T, N> y)
    {
        return unroll(std::multiplies<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline std::array<T, N> operator/(std::array<T, N> x, std::array<T, N> y)
    {
        return unroll(std::divides<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline std::array<T, N> max(std::array<T, N> x, std::array<T, N> y)
    {
        return unroll([](auto a, auto b) { return std::max(a, b); }, x, y);
    }

    template <typename T, std::size_t N>
    inline std::array<T, N> min(std::array<T, N> x, std::array<T, N> y)
    {
        return unroll([](auto a, auto b) { return std::min(a, b); }, x, y);
    }

    template <typename T, typename U, std::size_t N>
    inline std::array<T, N> cast(std::array<U, N> x)
    {
        return unroll([](auto a) { return static_cast<T>(a); }, x);
    }

    namespace detail {

        template <typename T, std::size_t N, std::size_t... Is>
        inline void store(std::array<T, N> x, T* array, std::index_sequence<Is...>)
        {
            std::tie(array[Is]...) = std::tie(x[Is]...);
        }

    } // namespace detail

    template <typename T, std::size_t N>
    inline void store(std::array<T, N> x, T* array)
    {
        detail::store(x, array, std::make_index_sequence<N> {});
    }

    namespace detail {

    template <std::size_t, typename T>
        constexpr T identity(T x) { return x; }

        template <typename, typename>
        struct scalar_impl;

        template <typename T, std::size_t N, std::size_t... Is>
        struct scalar_impl<std::array<T, N>, std::index_sequence<Is...>> {
          inline std::array<T, N> operator()(T x)
            {
                return { identity<Is>(x)... };
            }
        };

    } // namespace detail

    template <typename V>
    inline V scalar(typename V::value_type x)
    {
        return detail::scalar_impl<V, std::make_index_sequence<std::tuple_size<V>::value>> {}(x);
    }

    namespace detail {

        template <typename, typename>
        struct ascend_from_impl;

        template <typename T, std::size_t N, std::size_t... Is>
        struct ascend_from_impl<std::array<T, N>, std::index_sequence<Is...>> {
          inline std::array<T, N> operator()(T start, T step)
            {
                return { (start + step * Is)... };
            }
        };

    } // namespace detail

    template <typename V>
    inline V ascend_from(typename V::value_type start, typename V::value_type step)
    {
        return detail::ascend_from_impl<V, std::make_index_sequence<std::tuple_size<V>::value>> {}(start, step);
    }

} // namespace pure_simd

#endif /* PURE_SIMD_H */
