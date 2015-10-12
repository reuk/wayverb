cd build
cmake .. && make && ctest -V && cmd/pwaveguide ../demo/assets/test_models/vault.obj sndfile.aif
