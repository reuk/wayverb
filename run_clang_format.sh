#!/bin/zsh
ls lib/*.(h|cpp) | xargs clang-format -i
ls common/*.(h|cpp) | xargs clang-format -i
ls cmd/*.(h|cpp) | xargs clang-format -i
ls tests/*.(h|cpp) | xargs clang-format -i
