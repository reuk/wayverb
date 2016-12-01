#!/bin/sh

mkdir -p doxy_working
cd doxy_working
cmake -DONLY_BUILD_DOCS=ON .. && cmake --build .
cd -
rm -rf doxy_working
