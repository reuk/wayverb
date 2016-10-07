#include "common/sinc.h"

#include "frequency_domain/convolver.h"

#include "mesh_impulse_response.h"

namespace waveguide {

std::vector<float> make_transparent(const float* begin, const float* end) {
    //  window ir
    const auto input_size{std::distance(begin, end)};
    auto windowed{right_hanning(input_size)};
    elementwise_multiply(windowed, mesh_impulse_response);

    //  create convolver
    frequency_domain::convolver convolver{input_size + windowed.size() - 1};

    //  get convolved signal
    auto convolved{
            convolver.convolve(begin, end, windowed.begin(), windowed.end())};

    //  subtract from original kernel
    for (auto i = 0u; i != convolved.size(); ++i) {
        convolved[i] = (i < input_size ? begin[i] : 0) - convolved[i];
    }

    return convolved;
}

}  // namespace waveguide
