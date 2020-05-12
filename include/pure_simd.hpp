#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <functional>

namespace pure_simd {

    template <typename T, size_t N1>
    struct alignas(32) vector {
        using value_type = T;
        static constexpr size_t N = N1;
        constexpr T& operator[](size_t pos) { return data[pos]; }
        constexpr T operator[](size_t pos) const { return data[pos]; }
        T data[N];
    };

    namespace detail {

        template <typename Func, typename T, std::size_t N, std::size_t... Is>
        inline auto unroll(Func func, vector<T, N> x, vector<T, N> y, std::index_sequence<Is...>)
            -> vector<decltype(func(x[0], y[0])), N>
        {
            return { func(x[Is], y[Is])... };
        }

        template <typename Func, typename T, std::size_t N, std::size_t... Is>
        inline auto unroll(Func func, vector<T, N> x, std::index_sequence<Is...>)
            -> vector<decltype(func(x[0])), N>
        {
            return { func(x[Is])... };
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
    inline vector<T, N> operator+(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::plus<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline vector<T, N> operator-(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::minus<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline vector<T, N> operator-(vector<T, N> x)
    {
        return unroll(std::negate<T> {}, x);
    }

    template <typename T, std::size_t N>
    inline vector<T, N> operator*(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::multiplies<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline vector<T, N> operator/(vector<T, N> x, vector<T, N> y)
    {
        return unroll(std::divides<T> {}, x, y);
    }

    template <typename T, std::size_t N>
    inline vector<T, N> max(vector<T, N> x, vector<T, N> y)
    {
        return unroll([](auto a, auto b) { return std::max(a, b); }, x, y);
    }

    template <typename T, std::size_t N>
    inline vector<T, N> min(vector<T, N> x, vector<T, N> y)
    {
        return unroll([](auto a, auto b) { return std::min(a, b); }, x, y);
    }

    template <typename T, typename U, std::size_t N>
    inline vector<T, N> cast(vector<U, N> x)
    {
        return unroll([](auto a) { return static_cast<T>(a); }, x);
    }

    namespace detail {

        template <typename T, std::size_t N, std::size_t... Is>
        inline void store(vector<T, N> x, T* array, std::index_sequence<Is...>)
        {
            std::tie(array[Is]...) = std::tie(x[Is]...);
        }

    } // namespace detail

    template <typename T, std::size_t N>
    inline void store(vector<T, N> x, T* array)
    {
        detail::store(x, array, std::make_index_sequence<N> {});
    }

    namespace detail {

        template <std::size_t, typename T>
        constexpr T identity(T x) { return x; }

        template <typename, typename>
        struct scalar_impl;

        template <typename T, std::size_t N, std::size_t... Is>
        struct scalar_impl<vector<T, N>, std::index_sequence<Is...>> {
            inline vector<T, N> operator()(T x)
            {
                return { identity<Is>(x)... };
            }
        };

    } // namespace detail

    template <typename V>
    inline V scalar(typename V::value_type x)
    {
      return detail::scalar_impl<V, std::make_index_sequence<V::N>> {}(x);
    }

    namespace detail {

        template <typename, typename>
        struct ascend_from_impl;

        template <typename T, std::size_t N, std::size_t... Is>
        struct ascend_from_impl<vector<T, N>, std::index_sequence<Is...>> {
            inline vector<T, N> operator()(T start, T step)
            {
                return { (start + step * Is)... };
            }
        };

    } // namespace detail

    template <typename V>
    inline V ascend_from(typename V::value_type start, typename V::value_type step)
    {
      return detail::ascend_from_impl<V, std::make_index_sequence<V::N>> {}(start, step);
    }

} // namespace pure_simd

#endif /* PURE_SIMD_H */
