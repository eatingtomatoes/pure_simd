cmake_minimum_required(VERSION 3.0)

project(pure_simd VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -march=native")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(include benchmark)

add_executable(
  benchmark_pure_simd
  benchmark/main.cpp
  benchmark/simple_shader.cpp
  )

target_link_libraries(
  benchmark_pure_simd
  ${CONAN_LIBS_BENCHMARK}
  )

add_executable(
  test_pure_simd
  test/test_array.cpp
  test/test_tuple.cpp
  test/test_simple_shader.cpp
  benchmark/simple_shader.cpp  
  )

target_link_libraries(
  test_pure_simd
  ${CONAN_LIBS_GTEST}
  )

