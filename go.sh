#!/bin/zsh
cd build
cmake .. && make && ctest -V && cmd/pwaveguide ../demo/assets/test_models/vault.obj ../demo/assets/materials/vault.json sndfile.wav
