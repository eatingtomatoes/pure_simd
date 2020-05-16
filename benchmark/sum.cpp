#include <random>

#include "benchmark/benchmark.h"

#include "sum.hpp"

#define N 250000

namespace {
    inline namespace fixture {
        std::vector<std::vector<std::uint8_t>> bitValues;
        std::vector<std::vector<std::uint8_t>> byteValues;
        std::vector<std::vector<std::uint16_t>> wordValues;
        std::vector<std::vector<std::uint32_t>> longValues;
        std::vector<std::vector<float>> floatValues;
        std::vector<std::vector<double>> doubleValues;

        bool initialized = false;

        template <typename T>
        std::vector<T> random_vector_int(T base)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, base);
            std::vector<T> vec(N);
            std::generate(vec.begin(), vec.end(), [&] {
                return dis(gen);
            });
            return vec;
        }

        template <>
        std::vector<std::uint8_t> random_vector_int<std::uint8_t>(std::uint8_t base)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<std::uint16_t> dis(0, base);
            std::vector<std::uint8_t> vec(N);
            std::generate(vec.begin(), vec.end(), [&] {
                return static_cast<std::uint8_t>(dis(gen));
            });
            return vec;
        }

        template <typename T>
        std::vector<T> random_vector_float(T max = 1.0)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0, std::nextafter(max, std::numeric_limits<T>::max()));
            std::vector<T> vec(N);
            std::generate(vec.begin(), vec.end(), [&] {
                return dis(gen);
            });
            return vec;
        }

        std::vector<std::uint8_t> random_vector_bits()
        {
            static_assert((N % 8) == 0);

            auto bytes = random_vector_int<std::uint8_t>(100);
            std::vector<std::uint8_t> bit_v;
            bit_v.reserve(N / 8);

            auto byte = bytes.begin();

            for (std::size_t i = 0; i < N; i += 8) {
                std::uint8_t theBits = 0;
                for (int b = 0; b < 8; ++b)
                    theBits |= (*byte++ != 0) << b;

                bit_v.push_back(theBits);
            }
            return bit_v;
        }

        void init(std::size_t count)
        {
            bitValues.reserve(count);
            std::generate_n(std::back_inserter(bitValues), count, [] {
                return random_vector_bits();
            });

            byteValues.reserve(count);
            std::generate_n(std::back_inserter(byteValues), count, [] {
                return random_vector_int<std::uint8_t>(100);
            });

            wordValues.reserve(count);
            std::generate_n(std::back_inserter(wordValues), count, [] {
                return random_vector_int<std::uint16_t>(10000);
            });

            longValues.reserve(count);
            std::generate_n(std::back_inserter(longValues), count, [] {
                return random_vector_int<std::uint32_t>(1000000);
            });

            floatValues.reserve(count);
            std::generate_n(std::back_inserter(floatValues), count, [] {
                return random_vector_float<float>();
            });

            doubleValues.reserve(count);
            std::generate_n(std::back_inserter(doubleValues), count, [] {
                return random_vector_float<double>();
            });
        }
    }

#define DEFINE_BENCHMARK_FOR(func_name)                                 \
    template <typename Target>                                          \
    void BM_sum_##func_name(benchmark::State& state)                    \
    {                                                                   \
        if (!initialized) {                                             \
            initialized = true;                                         \
            init(100);                                                  \
        }                                                               \
                                                                        \
        for (auto _ : state) {                                          \
            std::vector<Target> sum(N);                                 \
                                                                        \
            for (const auto& bits : bitValues)                          \
                func_name##_bits(sum.data(), N, bits.data(), 1.0);       \
                                                                        \
            for (const auto& bytes : byteValues)                        \
                func_name(sum.data(), N, bytes.data(), 1.0 / 100.0);     \
                                                                        \
            for (const auto& words : wordValues)                        \
                func_name(sum.data(), N, words.data(), 1.0 / 10000.0);   \
                                                                        \
            for (const auto& longs : longValues)                        \
                func_name(sum.data(), N, longs.data(), 1.0 / 1000000.0); \
                                                                        \
            for (const auto& floats : floatValues)                      \
                func_name(sum.data(), N, floats.data(), 1.0);            \
                                                                        \
            for (const auto& doubles : doubleValues)                    \
                func_name(sum.data(), N, doubles.data(), 1.0);           \
                                                                        \
            auto result = std::accumulate(sum.begin(), sum.end(), 0.0); \
                                                                        \
            benchmark::DoNotOptimize(result);                           \
        }                                                               \
    }

#define BENCHMARK_FOR(func_name, target) \
    BENCHMARK_TEMPLATE(BM_sum_##func_name, target)->Unit(benchmark::kMillisecond)

    DEFINE_BENCHMARK_FOR(scalar_add);
    DEFINE_BENCHMARK_FOR(pure_simd_add);

    BENCHMARK_FOR(scalar_add, int);
    BENCHMARK_FOR(pure_simd_add, int);

    BENCHMARK_FOR(scalar_add, float);
    BENCHMARK_FOR(pure_simd_add, float);

    BENCHMARK_FOR(scalar_add, double);
    BENCHMARK_FOR(pure_simd_add, double);
}
