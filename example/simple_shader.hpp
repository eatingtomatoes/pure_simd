#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H

#include <cstddef>

constexpr std::size_t SCRWIDTH = 512;
constexpr std::size_t SCRHEIGHT = 512;

void automatic_tick(float scale, float* screen);

void intrinsic_tick(float scale, float* screen);

void pure_simd_tick(float scale, float* screen);

#endif /* SIMPLE_SHADER_H */
