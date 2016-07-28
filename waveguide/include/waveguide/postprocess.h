#pragma once

#include "waveguide/rectangular_waveguide.h"

#include "common/receiver_settings.h"

namespace waveguide {

aligned::vector<aligned::vector<float>> run_attenuation(
        const model::ReceiverSettings& receiver,
        const aligned::vector<rectangular_waveguide::run_step_output>& input,
        double waveguide_sample_rate);

}  // namespace waveguide
