cd build
cmake .. && make && valgrind ctest -V && cmd/pwaveguide ../demo/assets/test_models/vault.obj sndfile.wav
