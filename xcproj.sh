#!/bin/sh
# for debugging and stuff

mkdir -p xc
cd xc

cmake -GXcode -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 ..
