#include "waveguide/postprocess.h"
#include "waveguide/attenuator/hrtf.h"
#include "waveguide/attenuator/microphone.h"

#include "common/map_to_vector.h"

namespace waveguide {

template <model::ReceiverSettings::Mode mode>
aligned::vector<aligned::vector<float>> run_attenuation(
        const model::ReceiverSettings& receiver,
        const aligned::vector<run_step_output>& input,
        double waveguide_sample_rate);

template <>
aligned::vector<aligned::vector<float>>
run_attenuation<model::ReceiverSettings::Mode::microphones>(
        const model::ReceiverSettings& receiver,
        const aligned::vector<run_step_output>& input,
        double waveguide_sample_rate) {
    return map_to_vector(receiver.microphones, [&](const auto& i) {
        return attenuator::microphone{
                get_pointing(i.pointer, receiver.position), i.shape}
                .process(input);
    });
}

template <>
aligned::vector<aligned::vector<float>>
run_attenuation<model::ReceiverSettings::Mode::hrtf>(
        const model::ReceiverSettings& receiver,
        const aligned::vector<run_step_output>& input,
        double waveguide_sample_rate) {
    const auto channels = {hrtf_channel::left, hrtf_channel::right};
    return map_to_vector(channels, [&](const auto& i) {
        auto ret{attenuator::hrtf{
                get_pointing(receiver.hrtf, receiver.position),
                glm::vec3(0, 1, 0),
                i}.process(input)};
        return multiband_filter_and_mixdown(ret, waveguide_sample_rate);
    });
}

//----------------------------------------------------------------------------//

aligned::vector<aligned::vector<float>> run_attenuation(
        const model::ReceiverSettings& receiver,
        const aligned::vector<run_step_output>& input,
        double sample_rate) {
    switch (receiver.mode) {
        case model::ReceiverSettings::Mode::microphones:
            return run_attenuation<model::ReceiverSettings::Mode::microphones>(
                    receiver, input, sample_rate);
        case model::ReceiverSettings::Mode::hrtf:
            return run_attenuation<model::ReceiverSettings::Mode::hrtf>(
                    receiver, input, sample_rate);
    }
}

}  // namespace waveguide
