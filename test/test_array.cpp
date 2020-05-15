#include <algorithm>
#include <numeric>
#include <type_traits>

#include "pure_simd.hpp"
#include "gtest/gtest.h"

using namespace pure_simd;

using vec = array<int, 5>;

TEST(TestArray, Inteface)
{
    vec xs { 1, 2, 3, 4, 5 };

    EXPECT_EQ(xs.N, 5);

    EXPECT_EQ(xs[0], 1);
    EXPECT_EQ(xs[1], 2);
    EXPECT_EQ(xs[2], 3);
    EXPECT_EQ(xs[3], 4);
    EXPECT_EQ(xs[4], 5);

    EXPECT_EQ(std::accumulate(xs.begin(), xs.end(), 0), 15);

    auto t = to_tuple(xs);

    EXPECT_TRUE((std::is_same<decltype(t), tuple_n<int, 5>>::value));
    EXPECT_EQ(std::get<0>(t), 1);
    EXPECT_EQ(std::get<1>(t), 2);
    EXPECT_EQ(std::get<2>(t), 3);
    EXPECT_EQ(std::get<3>(t), 4);
    EXPECT_EQ(std::get<4>(t), 5);
}

TEST(TestArray, UnrollUnaryOperation)
{
    vec xs { 0, 1, 2, 3, 4 };

    auto ys = unroll([](int a) { return a + 1; }, xs);

    ASSERT_EQ(ys[0], 1);
    ASSERT_EQ(ys[1], 2);
    ASSERT_EQ(ys[2], 3);
    ASSERT_EQ(ys[3], 4);
    ASSERT_EQ(ys[4], 5);

    vec zs = unroll(ys, [](int a) { return a * 2; });

    ASSERT_EQ(zs[0], 2);
    ASSERT_EQ(zs[1], 4);
    ASSERT_EQ(zs[2], 6);
    ASSERT_EQ(zs[3], 8);
    ASSERT_EQ(zs[4], 10);
}

TEST(TestArray, UnrollBinaryOperation)
{
    vec xs { 0, 1, 2, 3, 4 };
    vec ys { 5, 6, 7, 8, 9 };

    vec zs = unroll([](int a, int b) { return a + b; }, xs, ys);

    EXPECT_EQ(zs[0], 5);
    EXPECT_EQ(zs[1], 7);
    EXPECT_EQ(zs[2], 9);
    EXPECT_EQ(zs[3], 11);
    EXPECT_EQ(zs[4], 13);

    vec ws = unroll(xs, ys, [](int a, int b) { return a * b; });

    EXPECT_EQ(ws[0], 0);
    EXPECT_EQ(ws[1], 6);
    EXPECT_EQ(ws[2], 14);
    EXPECT_EQ(ws[3], 24);
    EXPECT_EQ(ws[4], 36);

    array<bool, 5> bs = unroll(zs, ws, [](int a, int b) { return a < b; });

    EXPECT_EQ(bs[0], false);
    EXPECT_EQ(bs[1], false);
    EXPECT_EQ(bs[2], true);
    EXPECT_EQ(bs[3], true);
    EXPECT_EQ(bs[4], true);

    array<bool, 5> mask { true, false, false, true, false };
    vec ns = unroll(xs, mask, [](int a, bool b) {
        return a * b;
    });

    EXPECT_EQ(ns[0], 0);
    EXPECT_EQ(ns[1], 0);
    EXPECT_EQ(ns[2], 0);
    EXPECT_EQ(ns[3], 3);
    EXPECT_EQ(ns[4], 0);
}

template <typename Vec>
bool all_equal(Vec xs, Vec ys)
{
    auto zs = xs == ys;
    return std::all_of(zs.begin(), zs.end(), [](auto a) { return a; });
}

#define EXPECT_VEC_EQUAL(xs, ys) \
    EXPECT_TRUE(all_equal((xs), (ys)))

using bvec = array<bool, 5>;

TEST(TestArray, BinaryOperator)
{
    vec xs { 0, 1, 2, 3, 4 };
    vec ys { 5, 6, 7, 8, 9 };

    EXPECT_VEC_EQUAL((vec { 0 + 5, 1 + 6, 2 + 7, 3 + 8, 4 + 9 }), xs + ys);
    EXPECT_VEC_EQUAL((vec { 0 - 5, 1 - 6, 2 - 7, 3 - 8, 4 - 9 }), xs - ys);
    EXPECT_VEC_EQUAL((vec { 0 * 5, 1 * 6, 2 * 7, 3 * 8, 4 * 9 }), xs * ys);
    EXPECT_VEC_EQUAL((vec { 0, 1, 2, 3, 4 }), (xs * ys) / ys);
    EXPECT_VEC_EQUAL((vec { 5 % 3, 6 % 3, 7 % 3, 8 % 3, 9 % 3 }), ys % scalar<vec>(3));
    EXPECT_VEC_EQUAL((vec { 0 ^ 5, 1 ^ 6, 2 ^ 7, 3 ^ 8, 4 ^ 9 }), xs ^ ys);
    EXPECT_VEC_EQUAL((vec { 0 & 5, 1 & 6, 2 & 7, 3 & 8, 4 & 9 }), xs & ys);
    EXPECT_VEC_EQUAL((vec { 0 | 5, 1 | 6, 2 | 7, 3 | 8, 4 | 9 }), xs | ys);
    EXPECT_VEC_EQUAL((vec { ~0, ~1, ~2, ~3, ~4 }), ~xs);
    EXPECT_VEC_EQUAL((bvec { !0, !1, !2, !3, !4 }), !xs);
    EXPECT_VEC_EQUAL((bvec { 0 < 5, 1 < 6, 2 < 7, 3 < 8, 4 < 9 }), xs < ys);
    EXPECT_VEC_EQUAL((bvec { 0 > 5, 1 > 6, 2 > 7, 3 > 8, 4 > 9 }), xs > ys);
    EXPECT_VEC_EQUAL((vec { 0 << 5, 1 << 6, 2 << 7, 3 << 8, 4 << 9 }), xs << ys);
    EXPECT_VEC_EQUAL((vec { 0 >> 5, 1 >> 6, 2 >> 7, 3 >> 8, 4 >> 9 }), xs >> ys);
    EXPECT_VEC_EQUAL((bvec { 0 == 5, 1 == 6, 2 == 7, 3 == 8, 4 == 9 }), xs == ys);
    EXPECT_VEC_EQUAL((bvec { 0 != 5, 1 != 6, 2 != 7, 3 != 8, 4 != 9 }), xs != ys);
    EXPECT_VEC_EQUAL((bvec { 0 <= 5, 1 <= 6, 2 <= 7, 3 <= 8, 4 <= 9 }), xs <= ys);
    EXPECT_VEC_EQUAL((bvec { 0 >= 5, 1 >= 6, 2 >= 7, 3 >= 8, 4 >= 9 }), xs >= ys);
    EXPECT_VEC_EQUAL((bvec { 0 && 5, 1 && 6, 2 && 7, 3 && 8, 4 && 9 }), xs && ys);
    EXPECT_VEC_EQUAL((bvec { 0 || 5, 1 || 6, 2 || 7, 3 || 8, 4 || 9 }), xs || ys);

    auto is_even = [](int x) { return x % 2 == 0; };
    vec zs = unroll(is_even, xs) * ys;

    EXPECT_VEC_EQUAL(zs, (vec{ true * 5, false * 6, true * 7, false * 8, true * 9 }));

    vec as { 5, 8, 2, 9, 7};
    vec bs{ 2, 8, 3, 10, 6};
    vec cs = (as <= bs) * ys;

    EXPECT_VEC_EQUAL(cs, (vec{false * 5, true * 6, true * 7, true * 8, false * 9}));
}

TEST(TestArray, MinMax)
{
    vec xs { 5, 1, 2, 8, 4 };
    vec ys { 0, 6, 7, 3, 9 };

    EXPECT_VEC_EQUAL((vec { 0, 1, 2, 3, 4 }), min(xs, ys));
    EXPECT_VEC_EQUAL((vec { 5, 6, 7, 8, 9 }), max(xs, ys));
}

TEST(TestArray, Cast)
{
    vec xs { 0, 1, 0, 0, 1 };
    EXPECT_VEC_EQUAL((bvec { false, true, false, false, true }), cast_to<bool>(xs));
}

TEST(TestArray, StoreLoad)
{
    int nums[5] { 1, 2, 3, 4, 5 };

    vec xs = load_from<vec>(nums);

    ASSERT_EQ(xs[0], 1);
    ASSERT_EQ(xs[1], 2);
    ASSERT_EQ(xs[2], 3);
    ASSERT_EQ(xs[3], 4);
    ASSERT_EQ(xs[4], 5);

    int xnums[5] {};

    store_to(xs, xnums);

    ASSERT_EQ(xnums[0], 1);
    ASSERT_EQ(xnums[1], 2);
    ASSERT_EQ(xnums[2], 3);
    ASSERT_EQ(xnums[3], 4);
    ASSERT_EQ(xnums[4], 5);
}

TEST(TestArray, Scarlar)
{
    EXPECT_VEC_EQUAL((vec { 2, 2, 2, 2, 2 }), scalar<vec>(2));
}

TEST(TestArray, AscendFrom)
{
    EXPECT_VEC_EQUAL((vec { 1, 3, 5, 7, 9 }), (ascend_from<vec, int>(1, 2)));
}

TEST(TestArray, UnrollLopp)
{
    bool four_stride = false;
    bool two_stride = false;
    bool one_stride = false;

    int sum = 0;

    unroll_loop<int, 4>(0, 15, [&](auto stride, int i) {
        if (decltype(stride)::value == 4) {
            four_stride = true;
            sum += 4;
        } else if (decltype(stride)::value == 2) {
            two_stride = true;
            sum += 2;
        } else {
            EXPECT_EQ(decltype(stride)::value, 1);
            one_stride = true;
            sum += 1;
        }
    });

    EXPECT_EQ(sum, 4 * 3 + 2 * 1 + 1 * 1);
}
