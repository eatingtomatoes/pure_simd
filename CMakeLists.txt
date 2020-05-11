cmake_minimum_required(VERSION 3.0)

project(pure_simd VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -march=native")

include_directories(
  include
  )

add_executable(bench
  src/main.cpp
  src/shader.cpp
  )
