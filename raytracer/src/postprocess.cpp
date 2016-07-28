#include "raytracer/postprocess.h"

#include "raytracer/attenuator.h"

#include "common/dsp_vector_ops.h"
#include "common/hrtf_utils.h"
#include "common/map.h"

namespace raytracer {

int compute_optimum_reflection_number(float min_amp, float max_reflectivity) {
    return std::log(min_amp) / std::log(max_reflectivity);
}

aligned::vector<aligned::vector<aligned::vector<float>>> flatten_impulses(
        const aligned::vector<aligned::vector<AttenuatedImpulse>>& attenuated,
        float samplerate) {
    return map_to_vector(attenuated, [samplerate](const auto& i) {
        return flatten_impulses(i, samplerate);
    });
}

double pressure_to_intensity(double pressure, double Z = 400) {
    return pressure * pressure / Z;
}

double intensity_to_pressure(double intensity, double Z = 400) {
    return std::sqrt(intensity * Z);
}

/// Turn a collection of AttenuatedImpulses into a vector of 8 vectors, where
/// each of the 8 vectors represent sample values in a different frequency band.
aligned::vector<aligned::vector<float>> flatten_impulses(
        const aligned::vector<AttenuatedImpulse>& impulse, float samplerate) {
    const auto MAX_TIME_LIMIT = 20.0f;
    // Find the index of the final sample based on time and samplerate
    auto maxtime = std::min(
            std::accumulate(impulse.begin(),
                            impulse.end(),
                            0.0f,
                            [](auto i, auto j) { return std::max(i, j.time); }),
            MAX_TIME_LIMIT);

    const auto MAX_SAMPLE = round(maxtime * samplerate) + 1;

    //  Create somewhere to store the results.
    aligned::vector<aligned::vector<float>> flattened(
            sizeof(VolumeType) / sizeof(float),
            aligned::vector<float>(MAX_SAMPLE, 0));

    //  For each impulse, calculate its index, then add the impulse's volumes
    //  to the volumes already in the output array.
    for (const auto& i : impulse) {
        const auto SAMPLE = round(i.time * samplerate);
        if (SAMPLE < MAX_SAMPLE) {
            for (auto j = 0u; j != flattened.size(); ++j) {
                flattened[j][SAMPLE] += i.volume.s[j];
            }
        }
    }

    //  impulses are intensity levels, now we need to convert to pressure
    proc::for_each(flattened, [](auto& i) {
        proc::for_each(i, [](auto& j) {
            j = std::copysign(intensity_to_pressure(std::abs(j)), j);
        });
    });

    return flattened;
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

aligned::vector<float> flatten_filter_and_mixdown(
        const aligned::vector<AttenuatedImpulse>& input,
        double output_sample_rate) {
    return multiband_filter_and_mixdown(
            raytracer::flatten_impulses(input, output_sample_rate),
            output_sample_rate);
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
