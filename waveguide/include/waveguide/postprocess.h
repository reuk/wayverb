#pragma once

#include "waveguide/waveguide.h"

#include "common/receiver_settings.h"

namespace waveguide {

aligned::vector<aligned::vector<float>> run_attenuation(
        const model::ReceiverSettings& receiver,
        const aligned::vector<run_step_output>& input,
        double waveguide_sample_rate);

}  // namespace waveguide
