#include "compressed_waveguide.h"

#include "waveguide/make_transparent.h"

#include <cassert>
#include <cmath>

void verify_compensation_signal() {
    ComputeContext c;
    compressed_rectangular_waveguide_program program(c.context, c.device);
    compressed_rectangular_waveguide waveguide(program, 100);

    std::vector<float> input{1, 2, 3, 4, 5, 4, 3, 2, 1};
    auto output = waveguide.run_soft_source(make_transparent(input));
    auto lim = std::min(input.size(), output.size());

    auto ok = true;
    for (auto i = 0u; i != lim && ok; ++i) {
        if (0.001f < std::fabs(input[i] - output[i])) {
            ok = false;
        }
    }

    if (!ok) {
        throw std::runtime_error(
                "input and output *should* be about equal but they're not");
    }

    std::cout << std::endl;
    std::cout << "everything looks fine" << std::endl;
}

int main() {
    verify_compensation_signal();
}
