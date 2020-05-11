#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <tuple>
#include <functional>

namespace pure_simd {

    template <typename... Ts>
    using vector = std::tuple<Ts...>;

    namespace detail {

        template <typename T, std::size_t>
        using just_type_t = T;

        template <typename T, typename Is>
        struct vector_n_impl;

        template <typename T, std::size_t... Is>
        struct vector_n_impl<T, std::index_sequence<Is...>> {
            using type = vector<just_type_t<T, Is>...>;
        };
    
    } // namespace detail

    template <typename T, std::size_t N>
    using vector_n = typename detail::vector_n_impl<T, std::make_index_sequence<N>>::type;

    namespace detail {

        // Extract the head of a type sequence.
        template <typename T, typename...>
        struct first_type {
            using type = T;
        };

        template <typename... Ts>
        using first_type_t = typename first_type<Ts...>::type;

        template <typename Func, typename... Args, std::size_t... Is>
        inline auto unroll(Func func, vector<Args...> x, vector<Args...> y, std::index_sequence<Is...>)
        {
            return std::make_tuple(func(std::get<Is>(x), std::get<Is>(y))...);
        }

        template <typename Func, typename... Args, std::size_t... Is>
        inline auto unroll(Func func, vector<Args...> x, std::index_sequence<Is...>)
        {
            return std::make_tuple(func(std::get<Is>(x))...);
        }
    
    } // namespace detail

    template <typename Func, typename... Args>
    inline auto unroll(Func func, vector<Args...> x, vector<Args...> y)
    {
        return detail::unroll(func, x, y, std::make_index_sequence<sizeof...(Args)> {});
    }

    template <typename Func, typename... Args>
    inline auto unroll(Func func, vector<Args...> x)
    {
        return detail::unroll(func, x, std::make_index_sequence<sizeof...(Args)> {});
    }

    template <typename... Args>
    inline auto operator+(vector<Args...> x, vector<Args...> y)
    {
         return unroll(std::plus<detail::first_type_t<Args...>> {}, x, y);
    }

    template <typename... Args>
    inline auto operator-(vector<Args...> x, vector<Args...> y)
    {
         return unroll(std::minus<detail::first_type_t<Args...>> {}, x, y);
    }

    template <typename... Args>
    inline auto operator-(vector<Args...> x)
    {
         return unroll(std::negate<detail::first_type_t<Args...>> {}, x);
    }

    template <typename... Args>
    inline auto operator*(vector<Args...> x, vector<Args...> y)
    {
        return unroll(std::multiplies<detail::first_type_t<Args...>> {}, x, y);
    }

    template <typename... Args>
    inline auto operator/(vector<Args...> x, vector<Args...> y)
    {
        return unroll(std::divides<detail::first_type_t<Args...>> {}, x, y);
    }

    template <typename... Args>
    inline auto max(vector<Args...> x, vector<Args...> y)
    {
      return unroll([](auto a, auto b){ return std::max(a, b); }, x, y);
    }

    template <typename... Args>
    inline auto min(vector<Args...> x, vector<Args...> y)
    {
      return unroll([](auto a, auto b){ return std::min(a, b); }, x, y);
    }

    template <typename T, typename... Args>
    inline auto cast(vector<Args...> x)
    {
        return unroll([](auto a) { return static_cast<T>(a); }, x);
    }

    namespace detail {

        template <typename... Args, std::size_t... Is>
        inline void store(vector<Args...> x,
            first_type_t<Args...>* array,
            std::index_sequence<Is...>)
        {
          std::tie(array[Is]...) = x;
        }
    
    } // namespace detail

    template <typename... Args>
    inline void store(vector<Args...> x, detail::first_type_t<Args...>* array)
    {
        detail::store(x, array, std::make_index_sequence<sizeof...(Args)> {});
    }

    namespace detail {

    template <typename T>
        constexpr T identity(T x) { return x; }

        template <typename>
        struct scalar_impl;

        template <typename... Args>
        struct scalar_impl<vector<Args...>> {
            inline auto operator()(first_type_t<Args...> x)
            {
              return std::make_tuple(identity<Args>(x) ...);
            }
        };

    } // namespace detail

    template <typename V>
    inline auto scalar(std::tuple_element_t<0, V> x)
    {
      return detail::scalar_impl<V>{}(x);
    }

    namespace detail {

        template <typename, typename>
        struct ascend_from_impl;

        template <typename... Args, std::size_t... Is>
        struct ascend_from_impl<vector<Args...>, std::index_sequence<Is...>> {
            inline auto operator()(first_type_t<Args...> start, first_type_t<Args ...> step)
            {
              return std::make_tuple(identity<Args>(start + step * Is) ...);
            }
        };

    } // namespace detail

    template <typename V>
    inline auto ascend_from(std::tuple_element_t<0, V> start, std::tuple_element_t<0, V> step)
    {
      return detail::ascend_from_impl<V, std::make_index_sequence<std::tuple_size<V>::value>>{}(start, step);
    }

} // namespace pure_simd

#endif /* PURE_SIMD_H */
