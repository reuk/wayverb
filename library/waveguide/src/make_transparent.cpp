#include "common/fast_convolver.h"
#include "common/sinc.h"

#include "mesh_impulse_response.h"

namespace waveguide {

aligned::vector<float> make_transparent(const aligned::vector<float> &kernel) {
    //  window ir
    auto windowed{right_hanning(mesh_impulse_response.size())};
    elementwise_multiply(windowed, mesh_impulse_response);

    //  create convolver
    fast_convolver fast_convolver{kernel.size() + windowed.size() - 1};

    //  get convolved signal
    auto convolved{fast_convolver.convolve(kernel, windowed)};

    //  subtract from original kernel
    for (auto i = 0u; i != convolved.size(); ++i) {
        convolved[i] = (i < kernel.size() ? kernel[i] : 0) - convolved[i];
    }

    return convolved;
}

}  // namespace waveguide
