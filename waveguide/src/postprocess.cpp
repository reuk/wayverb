#include "waveguide/postprocess.h"
#include "waveguide/attenuator/hrtf.h"
#include "waveguide/attenuator/microphone.h"

#include "common/map_to_vector.h"

namespace waveguide {

aligned::vector<aligned::vector<float>> run_attenuation(
        const model::ReceiverSettings& receiver,
        const aligned::vector<run_step_output>& input,
        double waveguide_sample_rate) {
    switch (receiver.mode) {
        case model::ReceiverSettings::Mode::microphones: {
            attenuator::microphone attenuator;
            return map_to_vector(
                    receiver.microphones,
                    [&receiver, &input, &attenuator](const auto& i) {
                        return attenuator.process(
                                input,
                                get_pointing(i.pointer, receiver.position),
                                i.shape);
                    });
        }
        case model::ReceiverSettings::Mode::hrtf: {
            attenuator::hrtf attenuator;
            const auto channels = {hrtf_channel::left, hrtf_channel::right};
            return map_to_vector(
                    channels,
                    [&receiver, &input, &attenuator, waveguide_sample_rate](
                            const auto& i) {
                        auto ret = attenuator.process(
                                input,
                                get_pointing(receiver.hrtf, receiver.position),
                                glm::vec3(0, 1, 0),
                                i);
                        return multiband_filter_and_mixdown(
                                ret, waveguide_sample_rate);
                    });
        }
    }
}

}  // namespace waveguide
