#pragma once

#include "waveguide/postprocessor/microphone.h"

#include "common/model/receiver_settings.h"

namespace waveguide {

aligned::vector<aligned::vector<float>> run_attenuation(
        const model::receiver_settings& receiver,
        const aligned::vector<postprocessor::microphone_state::output>& input,
        double waveguide_sample_rate);

}  // namespace waveguide
