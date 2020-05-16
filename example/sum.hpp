#ifndef SUM_H
#define SUM_H

#include <algorithm>
#include <cstdint>
#include <numeric>

#include "pure_simd.hpp"


template <typename T, typename U>
void scalar_add(T* target, std::size_t n, const U* source, double factor)
{
    const auto theFactor = static_cast<T>(factor);

    std::transform(target, target + n, source, target, [theFactor](auto a, auto b) {
        return a + (b * theFactor);
    });
}

template <typename T>
void scalar_add_bits(T* target, std::size_t n, const std::uint8_t* source, double factor)
{
    // static_assert((n % 8) == 0);

    for (size_t i = 0; i < n; i += 8) {
        auto theBits = *source++;
        for (int b = 0; b < 8; ++b)
            *target++ += ((theBits >> b) & 0x01) * factor;
    }
}

// At present, the best size depends on your machine.
#define VECTOR_SIZE 16

template <typename T, typename U>
void pure_simd_add(T* target, std::size_t n, const U* source, double factor)
{
    const auto theFactor = static_cast<T>(factor);
    
    pure_simd::transform<VECTOR_SIZE>(target, n, source, target, [theFactor](auto a, auto b) {
        return a + b * theFactor;
    });
}

template <typename T>
void pure_simd_add_bits(T* target, std::size_t n, const std::uint8_t* source, double factor)
{
    // static_assert((n % 8) == 0);

    using TargetVec = pure_simd::vector<T, VECTOR_SIZE>;
    const auto theSummand = pure_simd::scalar<TargetVec>(static_cast<T>(factor));

    for (size_t i = 0; i < n; i += VECTOR_SIZE, target += VECTOR_SIZE) {
        auto value = pure_simd::load_from<TargetVec>(target);
        auto theBits = pure_simd::scatter_bits<TargetVec>(*source++);
        pure_simd::store_to(value + (theBits * theSummand), target);
    }
}

#endif /* SUM_H */
