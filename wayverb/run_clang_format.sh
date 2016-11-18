#!/bin/zsh

if [[ `uname` == 'Linux' ]]; then
    cf='clang-format-3.5'
else
    cf='clang-format'
fi

ls Source/**/*.(h|hpp|cpp) | xargs ${cf} -i
