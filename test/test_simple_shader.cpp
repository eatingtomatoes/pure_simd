#include <numeric>
#include <iostream>

#include "gtest/gtest.h"

#include "pure_simd.hpp"

#include "simple_shader.hpp"

TEST(TestSimpleShader, Intrinsic) {
    std::vector<float> buffer1(SCRWIDTH* SCRHEIGHT, 0.0);
    automatic_tick(0.6f, buffer1.data());

    std::vector<float> buffer2(SCRWIDTH* SCRHEIGHT, 0.0);
    intrinsic_tick(0.6f, buffer2.data());    

    float expected = std::accumulate(buffer1.begin(), buffer1.end(), 0.0f);
    float got = std::accumulate(buffer2.begin(), buffer2.end(), 0.0f);

    EXPECT_FLOAT_EQ(expected, got);
}

TEST(TestSimpleShader, PureSIMD) {
    std::vector<float> buffer1(SCRWIDTH* SCRHEIGHT, 0.0);
    automatic_tick(0.6f, buffer1.data());

    std::vector<float> buffer2(SCRWIDTH* SCRHEIGHT, 0.0);
    pure_simd_tick(0.6f, buffer2.data());    

    float expected = std::accumulate(buffer1.begin(), buffer1.end(), 0.0f);
    float got = std::accumulate(buffer2.begin(), buffer2.end(), 0.0f);

    EXPECT_FLOAT_EQ(expected, got);
}
