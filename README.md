# Pure SIMD 

A simple, extensible, portable, efficient and header-only SIMD library!

- [Pure SIMD](#pure-simd)
  
  * [Introduction](#introduction)
  * [Compiler Requirements](#compiler-requirements)
  * [Interface](#interface)
    + [Types](#types)
    + [Basic Constructs](#basic-constructs)
    + [High-level Operations](#high-level-operations) 
  * [Example](#example)
  * [Test and Benchmark](#test-and-benchmark)
  * [Development Status](#development-status)
  * [To do](#to-do)

<small><i><a href='http://ecotrust-canada.github.io/markdown-toc/'>Table of contents generated with markdown-toc</a></i></small>

## Introduction

There are already tons of SIMD libraries, which usually export user-friendly interfaces by wrapping
the underlying messy SIMD intrinsics. 

Pure SIMD also provides user-friendly interfaces, but by a different means. It just unrolls loops introduced by various vector operations at compile time and leaves the rest work to compilers. Modern compilers can generate SIMD instructions from them easily, then the vectorization is done. 

This simple idea brings the following **advantages** to Pure SIMD:

* **Simplicity**. The implementation uses variadic templates to unroll loops introduced by various vector operations and to construct user-friendly interfaces. Neither intrinsics nor extra library dependencies are required. If you known variadic templates, you can write your own version very quickly.

* **Extensibility**. You can use the basic constructs to easily implement various vector operations. Nothing will limits your hands.

* **Portability**. All codes are written in standard c++14 and there are no extra dependences and no intrinsics. Some hight-level vector operations might have no corresponding low-level instructions on your machine, but that doesn't matter. Your program will run normally and even performs better than scalar ones due to the benefits of loop unrolling.

* **Efficiency**. The Pure SIMD depends on compilers to generate SIMD instructions from unrolled loops. For compilers supporting SLP(superword level parallelism) vectorization, such as gcc and clang, it's not a problem. As long as your compiler is OK, you can get nearly the same assembly code as manually-vectorized ones. Furthermore, intrinsics might get in the way of compiler's optimizations, while Pure SIMD has no such problems. Thus the latter may lead to better performance.

* **Header-only**.  

## Compiler Requirements

C++17 & SLP vectorization.

## Interface

All definitions of types and functions sit in the namespace `pure_simd`.

### Types

#### array

Pure SIMD uses `vector`, which is an aligned version of std::array, to model a sequence of values. 

```c++
    template <typename T, std::size_t N, std::size_t Align = 32>
    struct alignas(Align) vector;
```

#### size_constant 

It's just an alias for convenience.

```c++
    template <size_t N>
    using size_constant = std::integral_constant<size_t, N>;
```

### Basic Constructs

The `unroll` function unrolls unary/binary operations on vectors. The result's type depends on the operations. You can use it to implement other operations.

```c++
    template <typename F, typename V, typename = must_be_vector<V>>
    constexpr auto unroll(F func, V xs);

     template <
        typename F, typename V0, typename V1,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = assert_same_size<V0, V1>>
    constexpr auto unroll(F func, V0 xs, V1 ys);
```

To facilitate the use of lambda,  two variants are provided.

```c++
    template <typename F, typename V, typename = must_be_vector<V>>
    constexpr auto unroll(V xs, F func);

    template <
        typename F, typename V0, typename V1,
        typename = must_be_vector<V0>,
        typename = must_be_vector<V1>,
        typename = assert_same_size<V0, V1>>
    constexpr auto unroll(V0 xs, V1 ys, F func);
```

So you can write code like this:

```c++
    auto zs = unroll(xs, ys, [](auto a, auto b) {
        return a * b;
    });
```

### High-level Operations

#### Arithmetic & Conversion Operations

Currently Pure SIMD supports +, -, *, /, %, ^, &, |, ~, !, <, >, <<,  >>, ==, !=, <=, >=, &&, ||, max, min, and cast operations.

Note that <, >, ==, !=, <= and >= are not defined for tuples, or they will conflict with those in the c++ standard library.

#### Load & Store Operation

The `store_to` writes a vector's elements to continuous locations.

```c++
    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr void store_to(V xs, T* dst)
```

The `load_from` reads values from continuous locations to a vector.

```c++
    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr V load_from(const T* src);
```

The `scalar_to` constructs a vector from a scalar value.

```c++
    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr V scalar(T x);
```

The `iota` constructs a vector of ascending sequence , that is, V{ start + step * 0, start + step * 1, ... }.

You can use a specific type for 0, 1 ... so as to avoid  unnecessary type conversion.

```c++
    template <
        typename V, typename I = size_t,
        typename T, typename S,
        typename = must_be_vector<V>>
    constexpr V iota(T start, S step);
```

#### Helpers for unrolling loops 

When the number of iterations is not a multiple of your vectors' size, extra code is need to handle the tail end. `unroll_loop` can do that for you.

`unroll_loop` decomposes a irregular loop into a series of subloops with successively halved steps and generates different loop bodies for them.

```c++
    template <typename S, S MaxStep, typename I, typename F>
    constexpr auto unroll_loop(I start, S iterations, F func)
        -> decltype(func(std::integral_constant<S, MaxStep> {}, start), void())
```

`func` should be a callable object or a generic lambda, as it will be used to generate bodies for subloops of different size at compile time. `func` will be passed two arguments. The first one tells you the step of current loop, which is usually used as vector size in that loop. The second one is the iteration index in the global loop. You may use it to access some data structure.

For example, suppose there is a loop of 0 up to 15, and you want to use vectors of size 4 to vectorize it, Then you write:

```c++
    // Use 4 as the maximum step.
    // `step` will get value of 4, 2 and 1 at compile time.
    // `i` will get value of  0, 4, 8, 12 and 14 at runtime.
    unroll_loop<int, 4>(0, 15, [&](auto step, int i) {
        constexpr std::size_t vector_size = decltype(step)::value;
        using fvec = vector<float, vector_size>;
         ...
    });
```

Then `unroll_loop` will generate three loops,  iterating from 0 to 12 with step of 4,  12 to 14 with step of 2, and 14 to 15 with step of 1.

At present,  the supported operations  are not enough, but it's easy to add new ones.

## Example

The following code comes from [Practical SIMD Programming](http://www.cs.uu.nl/docs/vakken/magr/2017-2018/files/SIMD%20Tutorial.pdf) with some modification for simplicity. It's quite well-optimized and very compute-intensive.

```c++
void automatic_tick(float scale, float* screen)
{
    for (int y = 0; y < SCRHEIGHT; y++) {
        float yoffs = 0.0f;
        float xoffs = 0.0f;
        float dx = scale / SCRWIDTH;

        for (int x = 0; x < SCRWIDTH; x++, xoffs += dx) {
            float ox = 0.0f;
            float oy = 0.0f;

            // The following loop dominates the function's performance.
            for (int i = 0; i < 99; i++) {
                float oy_x_oy = oy * oy;
                float ox_x_ox = ox * ox;
                float ox_x_oy = ox * oy;
                float oy_x_ox = oy * ox;

                oy = -(oy_x_oy - ox_x_ox - 0.55f + xoffs);
                ox = -(ox_x_oy + oy_x_ox - 0.55f + yoffs);
            }

            float r = std::min(255.0f, std::max(0.0f, ox * 255.0f));
            float g = std::min(255.0f, std::max(0.0f, oy * 255.0f));

            screen[x + y * SCRHEIGHT] = r + g;
        }
    }
}
```

Here is another version which manually vectorized using intrinsics. 

```c++
void intrinsic_tick(float scale, float* screen)
{
    for (int y = 0; y < SCRHEIGHT; y++) {
        float yoffs = 0.0;
        float xoffs = 0.0;
        float dx = scale / SCRWIDTH;

        for (int x = 0; x < SCRWIDTH; x += 4, xoffs += dx * 4) {
            __m128 ox4;
            __m128 oy4;

            ox4 = oy4 = _mm_setzero_ps();

            __m128 xoffs4 = _mm_setr_ps(
                xoffs,
                xoffs + dx,
                xoffs + dx * 2,
                xoffs + dx * 3 //
            );

            __m128 yoffs4 = _mm_set_ps1(yoffs);

            for (int i = 0; i < 99; i++) {
                __m128 px4 = ox4, py4 = oy4;

                oy4 = _mm_sub_ps(
                    _mm_setzero_ps(),
                    _mm_add_ps(_mm_sub_ps(
                                   _mm_sub_ps(
                                       _mm_mul_ps(py4, py4),
                                       _mm_mul_ps(px4, px4)),
                                   _mm_set_ps1(0.55f)),
                        xoffs4));

                ox4 = _mm_sub_ps(_mm_setzero_ps(),
                    _mm_add_ps(_mm_sub_ps(
                                   _mm_add_ps(_mm_mul_ps(px4, py4),
                                       _mm_mul_ps(py4, px4)),
                                   _mm_set_ps1(0.55f)),
                        yoffs4));
            }

            // The following loop is not vectorized.
            // But as it's outside of the hot loop, it doesn't affect the running time that much.
            // (I do want to vectorize it but I can't because of some problems caused by code simplification)
            for (int lane = 0; lane < 4; lane++) {
                float r = std::min(255.0, std::max(0.0, ox4[lane] * 255.0));
                float g = std::min(255.0, std::max(0.0, oy4[lane] * 255.0));

                screen[x + lane + y * SCRWIDTH] = r + g;
            }
        }
    }
}
```

The following code is the version rewritten with Pure SIMD. It's nearly identical to 
the original one, except for some type specifications.

```c++
void pure_simd_tick(float scale, float* screen)
{
    using namespace pure_simd;

    // We will use vectors of size 4, or size 4, 2 and 1 if SCRWIDTH is not a multiple of 4.
    constexpr std::size_t max_vector_size = 4;

    for (int y = 0; y < SCRHEIGHT; y++) {
        float yoffs = 0.0;
        float xoffs = 0.0;
        float dx = scale / SCRWIDTH;

        // unroll_loop will handle the tail end for us.
        unroll_loop<max_vector_size>(0, SCRWIDTH, [&](auto step, int x) {
            constexpr std::size_t vector_size = decltype(step)::value;
            using fvec = pure_simd::vector<float, vector_size>;
            
            fvec ox = scalar<fvec>(0.0f);
            fvec oy = scalar<fvec>(0.0f);

            for (int i = 0; i < 99; i++) {
                fvec oy_x_oy = oy * oy;
                fvec ox_x_ox = ox * ox;
                fvec ox_x_oy = ox * oy;
                fvec oy_x_ox = oy * ox;

                fvec xoffs4 = iota<fvec>(xoffs, dx);
                fvec yoffs4 = scalar<fvec>(yoffs);

                fvec dot_55 = scalar<fvec>(0.55f);

                oy = -(oy_x_oy - ox_x_ox - dot_55 + xoffs4);
                ox = -(ox_x_oy + oy_x_ox - dot_55 + yoffs4);
            }

            fvec r = pure_simd::min(
                scalar<fvec>(255.0f),
                pure_simd::max(scalar<fvec>(0.0f), ox * scalar<fvec>(255.0f)));

            fvec g = pure_simd::min(
                scalar<fvec>(255.0f),
                pure_simd::max(scalar<fvec>(0.0f), oy * scalar<fvec>(255.0f)));

            store_to(r + g, screen + x + y * SCRWIDTH);

            xoffs += max_vector_size * dx;
        });
    }
}
```

Here is the result of a benchmark, which was compiled  using clang++ 9.0 with -O3 and -march=native and executed on  Ubuntu 18.04 with Intel Core i7-9750H CPU, 

```
BM_automatic_tick_mean        114 ms 
BM_intrinsic_tick_mean            33.8 ms  
BM_pure_simd_tick_mean       27.4 ms 
```

You can see that the vectorized code using either Pure SIMD or intrinsics ran almost as three times faster as the scalar one. And the one using Pure SIMD is also a bit faster than the one using intrinsics. If you change the vector size from 4 to 32, the former will run as nine times faster as the latter! Thanks to the convenient interfaces, changing the size is quiet easy. 

Also, the vector size is not limited by the underlying machine, so you can enlarge it as much as you want. A major benefit of doing so is that you can exploit the power of different machines without change the code. The compiler will choose the best low-level vector size.

## Test and Benchmark

**Note** that the library is header-only, but Conan is needed to run the tests and benchmarks.

```shell
cd pure_simd
mkdir build && cd build
conan install ..
cmake ..
cmake --build .
./bin/test_pure_simd
./bin/benchmark_pure_simd
```

## Development Status

This library has just taken its first baby step. It's now in the experimental stage, so the interfaces often change drastically.

## To Do

* Add more operations  & documents 
* Add examples & benchmarks
* Keep consistencies across various compilers

