#include <vector>

#include <x86intrin.h>

#include "pure_simd.hpp"

#include "simple_shader.hpp"

void automatic_tick(float scale, float* screen)
{
    for (int y = 0; y < SCRHEIGHT; y++) {
        float yoffs = 0.0f;
        float xoffs = 0.0f;
        float dx = scale / SCRWIDTH;

        for (int x = 0; x < SCRWIDTH; x++, xoffs += dx) {
            float ox = 0.0f;
            float oy = 0.0f;

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

            screen[x + y * SCRWIDTH] = r + g;
        }
    }
}

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

            for (int lane = 0; lane < 4; lane++) {
                float r = std::min(255.0, std::max(0.0, ox4[lane] * 255.0));
                float g = std::min(255.0, std::max(0.0, oy4[lane] * 255.0));

                screen[x + lane + y * SCRWIDTH] = r + g;
            }
        }
    }
}

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
            using fvec = pure_simd::tuple_n<float, vector_size>;

            fvec ox = scalar<fvec>(0.0f);
            fvec oy = scalar<fvec>(0.0f);

            for (int i = 0; i < 99; i++) {
                fvec oy_x_oy = oy * oy;
                fvec ox_x_ox = ox * ox;
                fvec ox_x_oy = ox * oy;
                fvec oy_x_ox = oy * ox;

                fvec xoffs4 = ascend_from<fvec>(xoffs, dx);
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
