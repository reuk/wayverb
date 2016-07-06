#include "common/fast_convolution.h"

std::vector<float> make_transparent(const std::vector<float>& kernel) {
    //  open mesh impulse response
    //  TODO load mesh response somehow
    std::vector<float> ir(100, 0);
    ir.front() = 1.0;
    
    //  create convolver
    FastConvolver fast_convolver(kernel.size() + ir.size() - 1);
    return fast_convolver.convolve(kernel, ir);
}
