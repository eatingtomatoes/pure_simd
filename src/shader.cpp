#include <cmath>

#include "immintrin.h" // for AVX
#include "nmmintrin.h" // for SSE4.2

#include "driver_config.hpp"

#include "pure_simd.hpp"


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

void pure_simd_tick(float scale, float* screen)
{
  using namespace pure_simd;

  constexpr std::size_t vector_size = 4;
  using fvec = std::array<float, vector_size>;

    for (int y = 0; y < SCRHEIGHT; y++) {
        float yoffs = 0.0;
        float xoffs = 0.0;
        float dx = scale / SCRWIDTH;

        for (int x = 0; x < SCRWIDTH; x += vector_size, xoffs += vector_size * dx) {
            fvec ox = scalar<fvec>(0.0);
            fvec oy = scalar<fvec>(0.0);

            for (int i = 0; i < 99; i++) {
                fvec oy_x_oy = oy * oy;
                fvec ox_x_ox = ox * ox;
                fvec ox_x_oy = ox * oy;
                fvec oy_x_ox = oy * ox;

                fvec xoffs4 = ascend_from<fvec>(xoffs, dx);
                fvec yoffs4 = scalar<fvec>(yoffs);

                fvec dot_55 = scalar<fvec>(0.55);

                oy = -(oy_x_oy - ox_x_ox - dot_55 + xoffs4);
                ox = -(ox_x_oy + oy_x_ox - dot_55 + yoffs4);
            }

            fvec r = pure_simd::min(
                scalar<fvec>(255.0),
                pure_simd::max(scalar<fvec>(0.0), ox * scalar<fvec>(255.0)));

            fvec g = pure_simd::min(
                scalar<fvec>(255.0),
                pure_simd::max(scalar<fvec>(0.0), oy * scalar<fvec>(255.0)));

            store(r + g, screen + x + y * SCRWIDTH);
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
