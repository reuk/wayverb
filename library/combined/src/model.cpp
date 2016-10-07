#include "combined/model.h"

#include "utilities/aligned/vector.h"

namespace {
template <typename T>
float get_waveguide_sample_rate(T filter_frequency, T oversample_ratio) {
    return filter_frequency * oversample_ratio * (1 / 0.196);
}
}  // namespace

namespace model {

float get_waveguide_sample_rate(const App& a) {
    return ::get_waveguide_sample_rate(a.filter_frequency, a.oversample_ratio);
}

float get_waveguide_sample_rate(const SingleShot& a) {
    return ::get_waveguide_sample_rate(a.filter_frequency, a.oversample_ratio);
}

SingleShot get_single_shot(const App& a, size_t input, size_t output) {
    return SingleShot{a.filter_frequency,
                      a.oversample_ratio,
                      a.speed_of_sound,
                      a.rays,
                      a.source[input],
                      a.receiver_settings[output]};
}

aligned::vector<SingleShot> get_all_input_output_combinations(const App& a) {
    aligned::vector<SingleShot> ret;
    ret.reserve(a.source.size() * a.receiver_settings.size());
    for (const auto& i : a.source) {
        for (const auto& j : a.receiver_settings) {
            ret.emplace_back(SingleShot{a.filter_frequency,
                                        a.oversample_ratio,
                                        a.speed_of_sound,
                                        a.rays,
                                        i,
                                        j});
        }
    }
    return ret;
}

}  // namespace model
