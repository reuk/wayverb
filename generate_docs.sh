#!/bin/sh

mkdir -p doxy_output
cd doxy_output
cmake -DONLY_BUILD_DOCS=ON .. && cmake --build .
