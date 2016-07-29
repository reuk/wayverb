#include "raytracer/postprocess.h"

#include "raytracer/attenuator.h"

#include "common/dsp_vector_ops.h"
#include "common/map.h"

namespace raytracer {

int compute_optimum_reflection_number(float min_amp, float max_reflectivity) {
    return std::log(min_amp) / std::log(max_reflectivity);
}

double pressure_to_intensity(double pressure, double Z) {
    return pressure * pressure / Z;
}

double intensity_to_pressure(double intensity, double Z) {
    return std::copysign(std::sqrt(std::abs(intensity) * Z), intensity);
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
        double output_sample_rate) {
    return run_attenuation(
            cc, receiver, input.get_impulses(), output_sample_rate);
}

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const aligned::vector<Impulse>& input,
        double output_sample_rate) {
    switch (receiver.mode) {
        case model::ReceiverSettings::Mode::microphones: {
            raytracer::attenuator::microphone attenuator(cc.get_context(),
                                                         cc.get_device());
            return map_to_vector(receiver.microphones, [&](const auto& i) {
                return flatten_filter_and_mixdown(
                        attenuator.process(
                                input,
                                i.pointer.get_pointing(receiver.position),
                                i.shape,
                                receiver.position),
                        output_sample_rate);
            });
        }
        case model::ReceiverSettings::Mode::hrtf: {
            raytracer::attenuator::hrtf attenuator(cc.get_context(),
                                                   cc.get_device());
            const auto channels = {HrtfChannel::left, HrtfChannel::right};
            return map_to_vector(channels, [&](const auto& i) {
                return flatten_filter_and_mixdown(
                        attenuator.process(
                                input,
                                receiver.hrtf.get_pointing(receiver.position),
                                glm::vec3(0, 1, 0),
                                receiver.position,
                                i),
                        output_sample_rate);
            });
        }
    }
}

}  // namespace raytracer
