#!/bin/zsh
setopt extended_glob
ls waveguide/**/*.(h|cpp) | xargs clang-format -i
ls common/**/*.(h|cpp)~common/src/hrtf.cpp | xargs clang-format -i
ls tests/*.(h|cpp) | xargs clang-format -i
ls raytracer/**/*.(h|cpp) | xargs clang-format -i
ls tests/**/*.(h|cpp) | xargs clang-format -i
ls visualiser/Source/*.(h|hpp|cpp) | xargs clang-format -i
