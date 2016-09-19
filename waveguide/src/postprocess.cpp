#include "waveguide/postprocess.h"
#include "waveguide/attenuator/hrtf.h"
#include "waveguide/attenuator/microphone.h"

#include "common/map_to_vector.h"

namespace waveguide {

template <enum model::receiver_settings::mode mode>
aligned::vector<aligned::vector<float>> run_attenuation(
        const model::receiver_settings& receiver,
        const aligned::vector<postprocessor::microphone_state::output>& input,
        double waveguide_sample_rate);

template <>
aligned::vector<aligned::vector<float>>
run_attenuation<model::receiver_settings::mode::microphones>(
        const model::receiver_settings& receiver,
        const aligned::vector<postprocessor::microphone_state::output>& input,
        double waveguide_sample_rate) {
    return map_to_vector(receiver.microphones, [&](const auto& i) {
        return attenuator::microphone{
                get_pointing(i.orientable, receiver.position), i.shape}
                .process(input);
    });
}

template <>
aligned::vector<aligned::vector<float>>
run_attenuation<model::receiver_settings::mode::hrtf>(
        const model::receiver_settings& receiver,
        const aligned::vector<postprocessor::microphone_state::output>& input,
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
        const model::receiver_settings& receiver,
        const aligned::vector<postprocessor::microphone_state::output>& input,
        double sample_rate) {
    switch (receiver.mode) {
        case model::receiver_settings::mode::microphones:
            return run_attenuation<model::receiver_settings::mode::microphones>(
                    receiver, input, sample_rate);
        case model::receiver_settings::mode::hrtf:
            return run_attenuation<model::receiver_settings::mode::hrtf>(
                    receiver, input, sample_rate);
    }
}

}  // namespace waveguide
