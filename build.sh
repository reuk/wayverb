#!/bin/sh
# used by juce project to make sure all libraries are up-to-date

mkdir -p build
cd build
cmake -DCMAKE_CXX_CLANG_TIDY:STRING="clang-tidy;-checks=-*,clang-*,-clang-analyzer-alpha*,performance-*,readability-*" .. && cmake --build .
