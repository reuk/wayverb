#!/bin/sh

mkdir -p docs
cd docs
cmake -DONLY_BUILD_DOCS=ON .. && cmake --build .
