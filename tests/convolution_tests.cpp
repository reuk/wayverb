#include "common/filters_common.h"

#include "common/serialize/json_read_write.h"

#include "cereal/types/vector.hpp"

#include "gtest/gtest.h"

namespace std {
template <typename T>
JSON_OSTREAM_OVERLOAD(vector<T>);
}  // namespace std

TEST(convolution, convolution) {
    std::vector<float> a = {1, 0, 0, 0, 0};
    std::vector<float> b = {1, 2, 3, 4, 3, 2, 1, 0, 0};

    filter::FastConvolution fc(a.size() + b.size() - 1);
    auto convolved = fc.convolve(a, b);

    LOG(INFO) << convolved;
}
