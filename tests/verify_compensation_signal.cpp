#include "compressed_waveguide.h"

#include "waveguide/make_transparent.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cmath>

TEST(verify_compensation_signal, verify_compensation_signal) {
    ComputeContext c;
    compressed_rectangular_waveguide_program program(c.context, c.device);
    compressed_rectangular_waveguide waveguide(program, 100);

    std::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    auto output = waveguide.run_soft_source(make_transparent(input));

    auto lim = std::max(input.size(), output.size());
    for (auto i = 0u; i != lim; ++i) {
        auto in = i < input.size() ? input[i] : 0.0;
        auto out = i < output.size() ? output[i] : 0.0;
        ASSERT_TRUE(std::fabs(in - out) < 0.001f);
    }
}
