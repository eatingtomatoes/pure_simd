cmake_minimum_required(VERSION 3.0)

project(pure_simd VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -march=native")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(include example)

add_library(
  use_pure_simd
  example/shader.cpp
  )

add_executable(
  benchmark_pure_simd
  benchmark/main.cpp
  benchmark/shader.cpp
  benchmark/sum.cpp
  )

target_link_libraries(
  benchmark_pure_simd
  ${CONAN_LIBS_BENCHMARK}
  use_pure_simd
  )

add_executable(
  test_pure_simd
  test/vector.cpp
  test/shader.cpp
  test/sum.cpp
  )

target_link_libraries(
  test_pure_simd
  ${CONAN_LIBS_GTEST}
  use_pure_simd
  )

