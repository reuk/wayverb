#!/bin/sh
# used by juce project to make sure all libraries are up-to-date

mkdir -p build
cd build

clang_tidy_options=-DCMAKE_CXX_CLANG_TIDY:STRING="clang-tidy;-checks=-*,clang-*,-clang-analyzer-alpha*,performance-*,readability-*"

#iwyu_options=-DCMAKE_CXX_INCLUDE_WHAT_YOU_USE:STRING=iwyu

cmake .. && cmake --build .
