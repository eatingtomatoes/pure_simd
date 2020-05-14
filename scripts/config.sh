#!/usr/bin/env bash

mkdir -p build && cd build && cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/clang++-9
