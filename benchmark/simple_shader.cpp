#include "benchmark/benchmark.h"

#include "simple_shader.hpp"

#define BENCHMARK_FOR(func_name)                             \
    void BM_##func_name(benchmark::State& state)             \
    {                                                        \
        std::vector<float> screen(SCRWIDTH* SCRHEIGHT, 0.0); \
        float scale = 0.0;                                   \
                                                             \
        for (auto _ : state) {                               \
            scale += 0.1;                                    \
            func_name(scale, screen.data());                 \
        }                                                    \
    }                                                        \
    BENCHMARK(BM_##func_name) ->Unit(benchmark::kMillisecond)

BENCHMARK_FOR(automatic_tick);
BENCHMARK_FOR(intrinsic_tick);
BENCHMARK_FOR(pure_simd_tick);

