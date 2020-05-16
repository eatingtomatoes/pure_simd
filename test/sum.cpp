#include "gtest/gtest.h"

#include "sum.hpp"

#define TEST_FOR(func_name)          \
    TEST(TestSum, func_name)         \
    {                                \
        int xs[] { 0, 1, 2, 3, 4 };  \
        int ys[5] { 1, 2, 3, 4, 5 }; \
                                     \
        func_name(ys, 5, xs, 2);      \
                                     \
        EXPECT_EQ(ys[0], 1 + 0 * 2); \
        EXPECT_EQ(ys[1], 2 + 1 * 2); \
        EXPECT_EQ(ys[2], 3 + 2 * 2); \
        EXPECT_EQ(ys[3], 4 + 3 * 2); \
        EXPECT_EQ(ys[4], 5 + 4 * 2); \
    }

TEST_FOR(scalar_add)

TEST_FOR(pure_simd_add)


TEST(TestSum, ScalarAddBits) {
    std::uint8_t x = 0b01010111;
    int ys[8]{};

    scalar_add_bits(ys, 8, &x, 2);

    EXPECT_EQ(ys[0], 2);
    EXPECT_EQ(ys[1], 2);
    EXPECT_EQ(ys[2], 2);
    EXPECT_EQ(ys[3], 0);
    EXPECT_EQ(ys[4], 2);
    EXPECT_EQ(ys[5], 0);
    EXPECT_EQ(ys[6], 2);
    EXPECT_EQ(ys[7], 0);    
}

TEST(TestSum, PureSIMDAddBits) {
    std::uint8_t x = 0b01010111;
    int ys[8]{};

    pure_simd_add_bits(ys, 8, &x, 2);

    EXPECT_EQ(ys[0], 2);
    EXPECT_EQ(ys[1], 2);
    EXPECT_EQ(ys[2], 2);
    EXPECT_EQ(ys[3], 0);
    EXPECT_EQ(ys[4], 2);
    EXPECT_EQ(ys[5], 0);
    EXPECT_EQ(ys[6], 2);
    EXPECT_EQ(ys[7], 0);    
}
