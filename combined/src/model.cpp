#include "combined/model.h"

#include "common/aligned/vector.h"

namespace {
template <typename T>
float get_waveguide_sample_rate(T filter_frequency, T oversample_ratio) {
    return filter_frequency * oversample_ratio * (1 / 0.196);
}
}  // namespace

namespace model {

float App::get_waveguide_sample_rate() const {
    return ::get_waveguide_sample_rate(filter_frequency, oversample_ratio);
}

float SingleShot::get_waveguide_sample_rate() const {
    return ::get_waveguide_sample_rate(filter_frequency, oversample_ratio);
}

SingleShot App::get_single_shot(size_t input, size_t output) const {
    return SingleShot{filter_frequency,
                      oversample_ratio,
                      rays,
                      source[input],
                      receiver_settings[output]};
}

aligned::vector<SingleShot> App::get_all_input_output_combinations() const {
    aligned::vector<SingleShot> ret;
    ret.reserve(source.size() * receiver_settings.size());
    for (const auto& i : source) {
        for (const auto& j : receiver_settings) {
            ret.push_back(
                    SingleShot{filter_frequency, oversample_ratio, rays, i, j});
        }
    }
    return ret;
}

}  // namespace model
