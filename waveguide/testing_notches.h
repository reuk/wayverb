#pragma once

#include "rectangular_program.h"

namespace Testing {
std::array<RectangularProgram::FilterDescriptor,
           RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>
    notches{{
        RectangularProgram::FilterDescriptor{-60, 2000, 3},
        RectangularProgram::FilterDescriptor{-60, 4000, 3},
        RectangularProgram::FilterDescriptor{-60, 8000, 3},
    }};
}
