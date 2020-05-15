#include <algorithm>
#include <numeric>
#include <type_traits>

#include "pure_simd.hpp"
#include "gtest/gtest.h"

using namespace pure_simd;

using vec = tuple_n<int, 5>;

TEST(TestTuple, Inteface)
{
    vec xs { 1, 2, 3, 4, 5 };

    EXPECT_TRUE((std::is_same<vec, std::tuple<int, int, int, int, int>>::value));

    EXPECT_EQ(std::get<0>(xs), 1);
    EXPECT_EQ(std::get<1>(xs), 2);
    EXPECT_EQ(std::get<2>(xs), 3);
    EXPECT_EQ(std::get<3>(xs), 4);
    EXPECT_EQ(std::get<4>(xs), 5);

    auto t = to_array(xs);

    EXPECT_TRUE((std::is_same<decltype(t), array<int, 5>>::value));

    EXPECT_EQ(t[0], 1);
    EXPECT_EQ(t[1], 2);
    EXPECT_EQ(t[2], 3);
    EXPECT_EQ(t[3], 4);
    EXPECT_EQ(t[4], 5);

    tuple_n<bool, 5> mask { true, false, false, true, false };
    
    vec ns = unroll(xs, mask, [](int a, bool b) {
        return a * b;
    });

    EXPECT_EQ(std::get<0>(ns), 1);
    EXPECT_EQ(std::get<1>(ns), 0);
    EXPECT_EQ(std::get<2>(ns), 0);
    EXPECT_EQ(std::get<3>(ns), 4);
    EXPECT_EQ(std::get<4>(ns), 0);    
}

TEST(TestTuple, UnrollUnaryOperation)
{
    vec xs { 0, 1, 2, 3, 4 };

    auto ys = unroll([](int a) { return a + 1; }, xs);

    ASSERT_EQ(std::get<0>(ys), 1);
    ASSERT_EQ(std::get<1>(ys), 2);
    ASSERT_EQ(std::get<2>(ys), 3);
    ASSERT_EQ(std::get<3>(ys), 4);
    ASSERT_EQ(std::get<4>(ys), 5);

    vec zs = unroll(ys, [](int a) { return a * 2; });

    ASSERT_EQ(std::get<0>(zs), 2);
    ASSERT_EQ(std::get<1>(zs), 4);
    ASSERT_EQ(std::get<2>(zs), 6);
    ASSERT_EQ(std::get<3>(zs), 8);
    ASSERT_EQ(std::get<4>(zs), 10);
}

TEST(TestTuple, UnrollBinaryOperation)
{
    vec xs { 0, 1, 2, 3, 4 };
    vec ys { 5, 6, 7, 8, 9 };

    vec zs = unroll([](int a, int b) { return a + b; }, xs, ys);

    EXPECT_EQ(std::get<0>(zs), 5);
    EXPECT_EQ(std::get<1>(zs), 7);
    EXPECT_EQ(std::get<2>(zs), 9);
    EXPECT_EQ(std::get<3>(zs), 11);
    EXPECT_EQ(std::get<4>(zs), 13);

    vec ws = unroll(xs, ys, [](int a, int b) { return a * b; });

    EXPECT_EQ(std::get<0>(ws), 0);
    EXPECT_EQ(std::get<1>(ws), 6);
    EXPECT_EQ(std::get<2>(ws), 14);
    EXPECT_EQ(std::get<3>(ws), 24);
    EXPECT_EQ(std::get<4>(ws), 36);

    tuple_n<bool, 5> bs = unroll(zs, ws, [](int a, int b) { return a < b; });

    EXPECT_EQ(std::get<0>(bs), false);
    EXPECT_EQ(std::get<1>(bs), false);
    EXPECT_EQ(std::get<2>(bs), true);
    EXPECT_EQ(std::get<3>(bs), true);
    EXPECT_EQ(std::get<4>(bs), true);
}

template <typename Vec>
bool all_equal(Vec xs, Vec ys)
{
    auto zs = xs == ys;
    return std::all_of(zs.begin(), zs.end(), [](auto a) { return a; });
}

template <typename... Args>
bool all_equal(tuple<Args...> xs, tuple<Args...> ys)
{
    return all_equal(to_array(xs), to_array(ys));
}

#define EXPECT_VEC_EQUAL(xs, ys) \
    EXPECT_TRUE(all_equal((xs), (ys)))

using bvec = tuple_n<bool, 5>;

TEST(TestTuple, BinaryOperator)
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
    EXPECT_VEC_EQUAL((vec { 0 << 5, 1 << 6, 2 << 7, 3 << 8, 4 << 9 }), xs << ys);
    EXPECT_VEC_EQUAL((vec { 0 >> 5, 1 >> 6, 2 >> 7, 3 >> 8, 4 >> 9 }), xs >> ys);
    EXPECT_VEC_EQUAL((bvec { 0 && 5, 1 && 6, 2 && 7, 3 && 8, 4 && 9 }), xs && ys);
    EXPECT_VEC_EQUAL((bvec { 0 || 5, 1 || 6, 2 || 7, 3 || 8, 4 || 9 }), xs || ys);

    // These operators are defined in the c++ standard library.
    EXPECT_TRUE((std::is_same<bool, decltype(xs < ys)>::value));
    EXPECT_TRUE((std::is_same<bool, decltype(xs > ys)>::value));
    EXPECT_TRUE((std::is_same<bool, decltype(xs == ys)>::value));
    EXPECT_TRUE((std::is_same<bool, decltype(xs != ys)>::value));
    EXPECT_TRUE((std::is_same<bool, decltype(xs <= ys)>::value));
    EXPECT_TRUE((std::is_same<bool, decltype(xs >= ys)>::value));

    auto is_even = [](int x) { return x % 2 == 0; };
    vec zs = unroll(is_even, xs) * ys;

    EXPECT_VEC_EQUAL(zs, (vec { true * 5, false * 6, true * 7, false * 8, true * 9 }));


    auto le = [](auto a, auto b) { return a <= b; };

    vec as { 5, 8, 2, 9, 7 };
    vec bs { 2, 8, 3, 10, 6 };
    vec cs = unroll(le, as, bs) * ys;

    EXPECT_VEC_EQUAL(cs, (vec{false * 5, true * 6, true * 7, true * 8, false * 9}));
}

TEST(TestTuple, MinMax)
{
    vec xs { 5, 1, 2, 8, 4 };
    vec ys { 0, 6, 7, 3, 9 };

    EXPECT_VEC_EQUAL((vec { 0, 1, 2, 3, 4 }), pure_simd::min(xs, ys));
    EXPECT_VEC_EQUAL((vec { 5, 6, 7, 8, 9 }), pure_simd::max(xs, ys));
}

TEST(TestTuple, Cast)
{
    vec xs { 0, 1, 0, 0, 1 };
    EXPECT_VEC_EQUAL((bvec { false, true, false, false, true }), cast_to<bool>(xs));
}

TEST(TestTuple, StoreLoad)
{
    int nums[5] { 1, 2, 3, 4, 5 };

    vec xs = load_from<vec>(nums);

    ASSERT_EQ(std::get<0>(xs), 1);
    ASSERT_EQ(std::get<1>(xs), 2);
    ASSERT_EQ(std::get<2>(xs), 3);
    ASSERT_EQ(std::get<3>(xs), 4);
    ASSERT_EQ(std::get<4>(xs), 5);

    int xnums[5] {};

    store_to(xs, xnums);

    ASSERT_EQ(xnums[0], 1);
    ASSERT_EQ(xnums[1], 2);
    ASSERT_EQ(xnums[2], 3);
    ASSERT_EQ(xnums[3], 4);
    ASSERT_EQ(xnums[4], 5);
}

TEST(TestTuple, Scarlar)
{
    EXPECT_VEC_EQUAL((vec { 2, 2, 2, 2, 2 }), scalar<vec>(2));
}

TEST(TestTuple, AscendFrom)
{
    EXPECT_VEC_EQUAL((vec { 1, 3, 5, 7, 9 }), (ascend_from<vec, int>(1, 2)));
}

TEST(TestTuple, UnrollLopp)
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
