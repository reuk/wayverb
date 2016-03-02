#pragma once

#include "rectangular_program.h"

namespace Testing {
std::array<RectangularProgram::NotchFilterDescriptor,
           RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>
    notches{{
        RectangularProgram::NotchFilterDescriptor{-12, 500, 1},
        RectangularProgram::NotchFilterDescriptor{-12, 1200, 1},
        RectangularProgram::NotchFilterDescriptor{-12, 2400, 1},
    }};
}
