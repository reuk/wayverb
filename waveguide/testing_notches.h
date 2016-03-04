#pragma once

#include "rectangular_program.h"

namespace Testing {
std::array<RectangularProgram::NotchFilterDescriptor,
           RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>
    notches{{
        RectangularProgram::NotchFilterDescriptor{-60, 2000, 3},
        RectangularProgram::NotchFilterDescriptor{-60, 4000, 3},
        RectangularProgram::NotchFilterDescriptor{-60, 8000, 3},
    }};
}
