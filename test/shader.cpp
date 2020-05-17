#include "gtest/gtest.h"

#include "shader.hpp"

#define BUFFER_SIZE (SCRWIDTH * SCRHEIGHT)

TEST(TestShader, PureSIMD)
{
    std::vector<int> buffer(BUFFER_SIZE, 0);
    scalar_shader(2, buffer.data());

    for (int i = 1; i < BUFFER_SIZE; ++i) {
        EXPECT_NE(buffer[i], buffer[i - 1]);
    }

#define TEST_VECTOR_OF_SIZE(size)                   \
    std::vector<int> buffer##size(BUFFER_SIZE, 0);  \
                                                    \
    pure_simd_shader<size>(2, buffer##size.data()); \
                                                    \
    for (int i = 0; i < BUFFER_SIZE; ++i) {         \
        EXPECT_EQ(buffer[i], buffer##size[i]);      \
    }

    TEST_VECTOR_OF_SIZE(1)
    TEST_VECTOR_OF_SIZE(2)
    TEST_VECTOR_OF_SIZE(4)
    TEST_VECTOR_OF_SIZE(8)
    TEST_VECTOR_OF_SIZE(16)
    TEST_VECTOR_OF_SIZE(32)
    TEST_VECTOR_OF_SIZE(64)
    TEST_VECTOR_OF_SIZE(128)
}
