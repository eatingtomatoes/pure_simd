#include <vector>

#include <cmath>
#include <x86intrin.h>

#include "pure_simd.hpp"
#include "shader.hpp"

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

template <std::size_t MaxVectorSize>
void pure_simd_shader(int t, int* screen)
{
    namespace psd = pure_simd;
    
    for (int y = 0; y < SCRHEIGHT; ++y) {
        psd::unroll_loop<MaxVectorSize>(0, SCRWIDTH, [&](auto step, int x) {
            constexpr std::size_t vector_size = decltype(step)::value;
            using ivec = psd::vector<int, vector_size>;

            ivec ox = psd::scalar<ivec>(0);
            ivec oy = psd::scalar<ivec>(0);

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

template 
void pure_simd_shader<1>(int t, int* screen);

template 
void pure_simd_shader<2>(int t, int* screen);

template 
void pure_simd_shader<4>(int t, int* screen);

template 
void pure_simd_shader<8>(int t, int* screen);

template 
void pure_simd_shader<16>(int t, int* screen);

template 
void pure_simd_shader<32>(int t, int* screen);

template 
void pure_simd_shader<64>(int t, int* screen);

template 
void pure_simd_shader<128>(int t, int* screen);
