#!/bin/zsh
cd build
cmake .. && make && ctest -V && cmd/pwaveguide ~/dev/waveguide/demo/assets/test_models/vault.obj sndfile.aiff
