#!/bin/sh
# used by juce project to make sure all libraries are up-to-date

mkdir -p build
cd build
cmake .. && cmake --build .
