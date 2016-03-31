#!/bin/zsh
setopt extended_glob
ls waveguide/*.(h|cpp) | xargs clang-format -i
ls common/*.(h|cpp)~common/hrtf.cpp | xargs clang-format -i
ls cmd/*.(h|cpp) | xargs clang-format -i
ls tests/*.(h|cpp) | xargs clang-format -i
ls rayverb/*.(h|cpp) | xargs clang-format -i
ls tests/mic_test/*.(h|cpp) | xargs clang-format -i
ls tests/hybrid_test/*.(h|cpp) | xargs clang-format -i
ls tests/boundary_test/*.(h|cpp) | xargs clang-format -i
ls visualiser/Source/*.(h|hpp|cpp) | xargs clang-format -i
