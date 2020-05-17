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

* **Portability**. All codes are written in standard c++17 and there are no extra dependences and no intrinsics. Some hight-level vector operations might have no corresponding low-level instructions on your machine, but that doesn't matter. Your program will run normally and even performs better than scalar ones due to the benefits of loop unrolling.

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

#### Scatter & Gather Operations

`scatter_bits` constructs a vector from all bits of a scalar value. 

For instance, scatter_bits(0b01010111) => vector { 1, 1, 1, 0, 1, 0, 1, 0 }.

`gather_bits` does the opposite of `scatter_bits`.

```c++
    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr V scatter_bits(T bits);
    
    template <typename T, typename V, typename = must_be_vector<V>>
    constexpr T gather_bits(V xs);    
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

The following functions work in a way similar to the corresponding ones in the c++ standard library.

```c++
    template <typename V, typename T, typename = must_be_vector<V>>
    constexpr T sum(V x, T init);

    template <size_t VectorSize, typename F, typename T, typename S>
    constexpr void transform(const S* src, size_t n, T* dst, F func);

    template <size_t VectorSize, typename F, typename T, typename S0, typename S1>
    constexpr void transform(const S0* src0, size_t n, const S1* src1, T* dst, F func);

    template <size_t VectorSize, typename T, typename S, typename F>
    constexpr auto accumulate(const S* src, size_t n, T init, F func);

    template <size_t VectorSize, typename T, typename S>
    constexpr auto accumulate(const S* src, size_t n, T init);

    template <size_t VectorSize, typename T, typename S1, typename S2, typename FAdd, typename FMultiply>
    constexpr auto inner_product(const S1* src1, size_t n, const S2* src2, T init, FAdd f_add, FMultiply f_multiply);

    template <size_t VectorSize, typename T, typename S1, typename S2>
    constexpr auto inner_product(const S1* src1, size_t n, const S2* src2, T init);
```

At present,  the supported operations  are not enough, but it's easy to add new ones.

## Example

The following code comes from [Practical SIMD Programming](http://www.cs.uu.nl/docs/vakken/magr/2017-2018/files/SIMD%20Tutorial.pdf) with some modifications for simplicity and avoiding numeric errors. It's quite well-optimized and very compute-intensive.

```c++
void scalar_shader(int t, int* screen)
{
    for (int y = 0; y < SCRHEIGHT; ++y) {

        for (int x = 0; x < SCRWIDTH; ++x, ++t) {
            int ox = 0;
            int oy = 0;

            for (int i = 0; i < 99; ++i) {
                int px = ox;
                int py = oy;
                oy = -(py * py - px * px + t) % 10000079;
                ox = -(px * py + py * px - t) % 10000019;
            }

            screen[x + y * SCRHEIGHT] = ox + oy;
        }
    }
}
```

The following code is the version rewritten with Pure SIMD. It's nearly identical to 
the original one, except for some type specifications.

```c++
template <std::size_t MaxVectorSize>
void pure_simd_shader(int t, int* screen)
{
    namespace psd = pure_simd; 

    for (int y = 0; y < SCRHEIGHT; ++y) {
        // `unroll_loop` will handle the tail end.
        psd::unroll_loop<MaxVectorSize>(0, SCRWIDTH, [&](auto step, int x) {           
            constexpr std::size_t vector_size = decltype(step)::value;            
            ivec vt = psd::iota<ivec, int>(t, 1);

            for (int i = 0; i < 99; ++i) {
                ivec px = ox;
                ivec py = oy;

                oy = -(py * py - px * px + vt) % psd::scalar<ivec>(10000079);
                ox = -(px * py + py * px - vt) % psd::scalar<ivec>(10000019);
            }

            psd::store_to(ox + oy, screen + x + y * SCRHEIGHT);

            t += vector_size;
        });
    }
}
```

Here is the result of a benchmark, which used clang++ 9.0 with -O3 and -march=native and executed on  Ubuntu 18.04 with Intel Core i7-9750H CPU.  `pure_simd_shader` was tested with vectors of size 1, 2, ..., 128.

```
--------------------------------------------------------------------------------
Benchmark                                      Time             CPU   Iterations
--------------------------------------------------------------------------------
BM_shader_scalar_shader_mean                 103 ms          103 ms           10
BM_shader_pure_simd_shader_1_mean            103 ms          103 ms           10
BM_shader_pure_simd_shader_2_mean           61.3 ms         61.3 ms           10
BM_shader_pure_simd_shader_4_mean           50.3 ms         50.3 ms           10
BM_shader_pure_simd_shader_8_mean           28.0 ms         28.0 ms           10
BM_shader_pure_simd_shader_16_mean          16.2 ms         16.2 ms           10
BM_shader_pure_simd_shader_32_mean          12.5 ms         12.5 ms           10
BM_shader_pure_simd_shader_64_mean          10.9 ms         10.9 ms           10
BM_shader_pure_simd_shader_128_mean          158 ms          158 ms           10
```

You can see that as the vector size increased, the code using Pure SIMD was faster and faster. 

Generally speaking, the larger the size of vectors you use, the better performance you will get. But it's not a silver bullet. Too large unrolling factor will hurt the instruction cache.

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

