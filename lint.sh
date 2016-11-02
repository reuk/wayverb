mkdir -p lint
cd lint
cmake -DCMAKE_CXX_CLANG_TIDY:STRING="clang-tidy;-checks=-*,readability-*" .. && cmake --build .
