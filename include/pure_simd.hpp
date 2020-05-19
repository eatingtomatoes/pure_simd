#ifndef PURE_SIMD_H
#define PURE_SIMD_H

#include <climits>
#include <functional>

namespace pure_simd {
#if __AVX512BW__ | __AVX512CD__ | __AVX512DQ__ | __AVX512F__ | __AVX512VL__
    constexpr int register_size_bits = 512;
#elif __AVX__
    constexpr int register_size_bits = 256;
#else
    constexpr int register_size_bits = 128;
#endif

    constexpr int register_size = register_size_bits / CHAR_BIT;

    template<typename T, typename... Ts>
    constexpr int native_vectorsize() { return register_size / std::max({0UL, sizeof(T), (sizeof(Ts))...}); }


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
        static constexpr size_t align() { return Align; }

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

        template <typename F, typename V0, typename V1, typename V2, size_t... Is>
        constexpr auto unroll_impl(F func, V0 xs, V1 ys, V2 zs, std::index_sequence<Is...>)
            -> typename V0::template with_value_t<decltype(func(xs[0], ys[0], zs[0]))>
        {
            return { func(xs[Is], ys[Is], zs[Is])... };
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

    template <
        typename F, typename V0, typename V1, typename V2,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = must_be_vector<V2>,
        typename = assert_same_size<V0, V1>,
        typename = assert_same_size<V0, V2>>
    constexpr auto unroll(F func, V0 xs, V1 ys, V2 zs)
    {
        return detail::unroll_impl(func, xs, ys, zs, index_sequence_of<V0> {});
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

    template <
        typename F, typename V0, typename V1, typename V2,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = must_be_vector<V2>,
        typename = assert_same_size<V0, V1>,
        typename = assert_same_size<V0, V2>>
    constexpr auto unroll(V0 xs, V1 ys, V2 zs, F func)
    {
        return detail::unroll_impl(func, xs, ys, zs, index_sequence_of<V0> {});
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
    constexpr V abs(V xs)
    {
        return unroll(xs, [](auto a) { return std::abs(a); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr V ceil(V xs)
    {
        return unroll(xs, [](auto a) { return std::ceil(a); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr V floor(V xs)
    {
        return unroll(xs, [](auto a) { return std::floor(a); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr V round(V xs)
    {
        return unroll(xs, [](auto a) { return std::round(a); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr auto lround(V xs)
    {
        return unroll(xs, [](auto a) { return std::lround(a); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr auto llround(V xs)
    {
        return unroll(xs, [](auto a) { return std::llround(a); });
    }

    template <typename V, typename = must_be_vector<V>>
    constexpr V trunc(V xs)
    {
        return unroll(xs, [](auto a) { return std::trunc(a); });
    }

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

    template <typename V, typename = must_be_vector<V>>
    constexpr V clamp(V xs, typename V::value_type lo, typename V::value_type hi)
    {
        return unroll(xs, [lo, hi](auto a) { return std::clamp(a, lo, hi); });
    }

    template <
        typename V0, typename V1, typename V2,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = must_be_vector<V2>,
        typename = assert_same_size<V0, V1>,
        typename = assert_same_size<V0, V2>>
    constexpr auto multiply_add(V0 as, V1 bs, V2 cs)
    {
        return unroll(as, bs, cs, [](auto a, auto b, auto c) { return (a * b) + c; });
    }

    template <typename T, typename V, typename = must_be_vector<V>>
    constexpr auto cast_to(V xs)
    {
        return unroll(xs, [](auto a) { return static_cast<T>(a); });
    }

    namespace detail {
        template <typename V, typename VIdx, size_t... Is>
        constexpr auto permute_impl(V xs, VIdx idxs, std::index_sequence<Is...>)
            -> vector<typename V::value_type, VIdx::size(), V::align()>
        {
            return { (xs[idxs[Is]])... };
        }

    } // namespace detail

    template <
        typename V, typename VIdx,
        typename = must_be_vector<V>,
        typename = must_be_vector<VIdx>>
    constexpr auto permute(V xs, VIdx idxs)
    {
        return detail::permute_impl(xs, idxs, index_sequence_of<VIdx> {});
    }

    namespace detail {
        template <typename Vselect, typename V, size_t... Is>
        constexpr auto select_impl(Vselect vsel, std::array<V, 2> va, std::index_sequence<Is...>)
            -> vector<typename V::value_type, Vselect::size(), V::align()>
        {
            return { (va[vsel[Is]][Is])... };
        }

    } // namespace detail

    template <
        typename VSelect, typename V,
        typename = must_be_vector<VSelect>,
        typename = must_be_vector<V>>
    constexpr auto select(VSelect vs, V xs, V ys)
    {
        return detail::select_impl(vs, std::array<V, 2>({xs, ys}), index_sequence_of<VSelect> {});
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

    // Operations: sum.
    namespace detail {
        template <typename V, typename T, size_t... Is>
        constexpr T sum_impl(V x, T init, std::index_sequence<Is...>)
        {
            return (init + ... + x[Is]);
        }
    } // namespace detail

    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr T sum(V x, T init)
    {
        return detail::sum_impl(x, init, index_sequence_of<V> {});
    }

    // Algorithms: transform, accumulate, and inner_product.
    template <size_t VectorSize, typename F, typename T, typename S>
    constexpr void transform(const S* src, size_t n, T* dst, F func)
    {
        using SourceVec = pure_simd::vector<S, VectorSize>;

        auto rem = n % VectorSize;
        auto loops = n / VectorSize;

        for (size_t i = 0; i < loops; ++i, src += VectorSize, dst += VectorSize)
            store_to(unroll(load_from<SourceVec>(src), func), dst);

        std::transform(src, src + rem, dst, func);
    }

    template <size_t VectorSize, typename F, typename T, typename S0, typename S1>
    constexpr void transform(const S0* src0, size_t n, const S1* src1, T* dst, F func)
    {
        using Source0Vec = pure_simd::vector<S0, VectorSize>;
        using Source1Vec = pure_simd::vector<S1, VectorSize>;

        auto rem = n % VectorSize;
        auto loops = n / VectorSize;

        for (size_t i = 0; i < loops; ++i, src0 += VectorSize, src1 += VectorSize, dst += VectorSize)
            store_to(unroll(load_from<Source0Vec>(src0), load_from<Source1Vec>(src1), func), dst);

        std::transform(src0, src0 + rem, src1, dst, func);
    }

    template <size_t VectorSize, typename T, typename S, typename F>
    constexpr auto accumulate(const S* src, size_t n, T init, F func)
    {
        using Source = vector<S, VectorSize>;
        using Target = vector<T, VectorSize>;

        auto rem = n % VectorSize;
        auto loops = n / VectorSize;

        auto sum = scalar<Target>(init);
        for (size_t i = 0; i < loops; ++i, src += VectorSize)
            sum = unroll(sum, load_from<Source>(src), func);

        std::transform(src, src + rem, sum.begin(), sum.begin(), [&](auto val, auto curr) {
            return func(curr, val);
        });

        return sum;
    }

    template <size_t VectorSize, typename T, typename S>
    constexpr auto accumulate(const S* src, size_t n, T init)
    {
        return accumulate<VectorSize>(src, n, init, std::plus<>());
    }

    template <size_t VectorSize, typename T, typename S1, typename S2, typename FAdd, typename FMultiply>
    constexpr auto inner_product(const S1* src1, size_t n, const S2* src2, T init, FAdd f_add, FMultiply f_multiply)
    {
        using Source1 = vector<S1, VectorSize>;
        using Source2 = vector<S2, VectorSize>;
        using Target = vector<T, VectorSize>;

        auto rem = n % VectorSize;
        auto loops = n / VectorSize;

        auto sum = scalar<Target>(init);
        for (size_t i = 0; i < loops; ++i, src1 += VectorSize, src2 += VectorSize)
            sum = unroll(sum, unroll(load_from<Source1>(src1), load_from<Source2>(src2), f_multiply), f_add);

        auto tar = sum.begin();
        for (size_t i = 0; i < rem; ++i, ++src1, ++src2, ++tar)
            *tar = f_add(*tar, f_multiply(*src1, *src2));

        return sum;
    }

    template <size_t VectorSize, typename T, typename S1, typename S2>
    constexpr auto inner_product(const S1* src1, size_t n, const S2* src2, T init)
    {
        return inner_product<VectorSize>(src1, n, src2, init, std::plus<>(), std::multiplies<>());
    }

} // namespace pure_simd

#endif /* PURE_SIMD_H */
