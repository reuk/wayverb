#include "raytracer/postprocess.h"

#include "raytracer/attenuator.h"

#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"

namespace raytracer {

int compute_optimum_reflection_number(float min_amp, float max_reflectivity) {
    return std::log(min_amp) / std::log(max_reflectivity);
}

/// Find the index of the last sample with an amplitude of minVol or higher,
/// then resize the vectors down to this length.
void trimTail(aligned::vector<aligned::vector<float>>& audioChannels,
              float minVol) {
    using index_type = std::common_type_t<
            std::iterator_traits<
                    aligned::vector<float>::reverse_iterator>::difference_type,
            int>;

    // Find last index of required amplitude or greater.
    auto len = proc::accumulate(
            audioChannels, 0, [minVol](auto current, const auto& i) {
                return std::max(
                        index_type{current},
                        index_type{
                                distance(i.begin(),
                                         std::find_if(i.rbegin(),
                                                      i.rend(),
                                                      [minVol](auto j) {
                                                          return std::abs(j) >=
                                                                 minVol;
                                                      })
                                                 .base()) -
                                1});
            });

    // Resize.
    for (auto&& i : audioChannels)
        i.resize(len);
}

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const raytracer::results& input,
        double output_sample_rate, double acoustic_impedance) {
    return run_attenuation(cc,
                           receiver,
                           input.get_impulses(),
                           output_sample_rate,
                           input.get_speed_of_sound(), acoustic_impedance);
}

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const aligned::vector<impulse>& input,
        double output_sample_rate,
        double speed_of_sound,
        double acoustic_impedance) {
    switch (receiver.mode) {
        case model::ReceiverSettings::Mode::microphones: {
            raytracer::attenuator::microphone attenuator{cc, speed_of_sound};
            return map_to_vector(receiver.microphones, [&](const auto& i) {
                return flatten_filter_and_mixdown(
                        attenuator.process(
                                input,
                                get_pointing(i.pointer, receiver.position),
                                i.shape,
                                receiver.position),
                        output_sample_rate,
                        acoustic_impedance);
            });
        }
        case model::ReceiverSettings::Mode::hrtf: {
            raytracer::attenuator::hrtf attenuator{cc, speed_of_sound};
            const auto channels = {hrtf_channel::left, hrtf_channel::right};
            return map_to_vector(channels, [&](const auto& i) {
                return flatten_filter_and_mixdown(
                        attenuator.process(
                                input,
                                get_pointing(receiver.hrtf, receiver.position),
                                glm::vec3(0, 1, 0),
                                receiver.position,
                                i),
                        output_sample_rate,
                        acoustic_impedance);
            });
        }
    }
}

}  // namespace raytracer
