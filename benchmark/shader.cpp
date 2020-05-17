#include "benchmark/benchmark.h"

#include "shader.hpp"

void BM_shader_scalar_shader(benchmark::State& state)
{
    std::vector<int> buffer(SCRWIDTH * SCRHEIGHT, 0);

    for (auto _ : state) {
        scalar_shader(2, buffer.data());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_shader_scalar_shader)->Unit(benchmark::kMillisecond);

#define BENCHMARK_FOR(func_name, size)                           \
    void BM_shader_##func_name##_##size(benchmark::State& state) \
    {                                                            \
        std::vector<int> buffer(SCRWIDTH* SCRHEIGHT, 0);         \
                                                                 \
        for (auto _ : state) {                                   \
            func_name<size>(2, buffer.data());                   \
            benchmark::ClobberMemory();                          \
        }                                                        \
    }                                                            \
    BENCHMARK(BM_shader_##func_name##_##size)->Unit(benchmark::kMillisecond)

BENCHMARK_FOR(pure_simd_shader, 1);

BENCHMARK_FOR(pure_simd_shader, 2);

BENCHMARK_FOR(pure_simd_shader, 4);

BENCHMARK_FOR(pure_simd_shader, 8);

BENCHMARK_FOR(pure_simd_shader, 16);

BENCHMARK_FOR(pure_simd_shader, 32);

BENCHMARK_FOR(pure_simd_shader, 64);

BENCHMARK_FOR(pure_simd_shader, 128);
