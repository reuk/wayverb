#include "common/fast_convolver.h"

#include "common/sinc.h"
#include "data.h"

#include <algorithm>
#include <cassert>

std::vector<float> make_transparent(const std::vector<float> &kernel) {
    //  get ir data
    std::vector<float> ir(
            mesh_impulse_response::data,
            mesh_impulse_response::data + mesh_impulse_response::size);

    //  window ir
    auto window = right_hanning(ir.size());
    elementwise_multiply(ir, window);

    //  create convolver
    FastConvolver fast_convolver(kernel.size() + ir.size() - 1);

    //  return convolved signal
    return fast_convolver.convolve(kernel, ir);
}
