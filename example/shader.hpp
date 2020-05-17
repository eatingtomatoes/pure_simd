#ifndef SHADER_H
#define SHADER_H

#include <cstddef>

constexpr std::size_t SCRWIDTH = 512;
constexpr std::size_t SCRHEIGHT = 512;

void scalar_shader(int t, int* screen);

template <std::size_t MaxVectorSize>
void pure_simd_shader(int t, int* screen);

extern template 
void pure_simd_shader<1>(int t, int* screen);

extern template 
void pure_simd_shader<2>(int t, int* screen);

extern template 
void pure_simd_shader<4>(int t, int* screen);

extern template 
void pure_simd_shader<8>(int t, int* screen);

extern template 
void pure_simd_shader<16>(int t, int* screen);

extern template 
void pure_simd_shader<32>(int t, int* screen);

extern template 
void pure_simd_shader<64>(int t, int* screen);

extern template 
void pure_simd_shader<128>(int t, int* screen);

#endif /* SHADER_H */
