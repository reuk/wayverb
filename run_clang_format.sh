#!/bin/zsh
ls lib/*.(h|cpp) | xargs clang-format -i
ls src/*.(h|cpp) | xargs clang-format -i
ls tests/*.(h|cpp) | xargs clang-format -i
