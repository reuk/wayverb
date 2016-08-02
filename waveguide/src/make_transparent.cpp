#include "common/fast_convolver.h"

#include "common/sinc.h"
#include "data.h"

namespace waveguide {

aligned::vector<float> make_transparent(const aligned::vector<float> &kernel) {
    //  get ir data
    aligned::vector<float> ir(
            mesh_impulse_response::data,
            mesh_impulse_response::data + mesh_impulse_response::size);

    //  window ir
    auto window = right_hanning(ir.size());
    elementwise_multiply(ir, window);

    //  create convolver
    FastConvolver fast_convolver(kernel.size() + ir.size() - 1);

    //  get convolved signal
    auto convolved = fast_convolver.convolve(kernel, ir);

    //  subtract from original kernel
    for (auto i = 0u; i != convolved.size(); ++i) {
        convolved[i] = (i < kernel.size() ? kernel[i] : 0) - convolved[i];
    }

    return convolved;
}

}  // namespace waveguide
