#include "waveguide_config.h"
#include "samplerate.h"

float WaveguideConfig::get_max_frequency() const {
    return get_filter_frequency() * get_oversample_ratio();
}

float& WaveguideConfig::get_oversample_ratio() {
    return oversample_ratio;
}

float& WaveguideConfig::get_filter_frequency() {
    return filter_frequency;
}

float WaveguideConfig::get_waveguide_sample_rate() const {
    return get_max_frequency() * 4;
}

float WaveguideConfig::get_divisions() const {
    return (SPEED_OF_SOUND * sqrt(3)) / get_waveguide_sample_rate();
}

float WaveguideConfig::get_oversample_ratio() const {
    return oversample_ratio;
}
float WaveguideConfig::get_filter_frequency() const {
    return filter_frequency;
}

std::vector<float> adjust_sampling_rate(std::vector<float>& w_results,
                                        const WaveguideConfig& cc) {
    std::vector<float> out_signal(cc.get_output_sample_rate() *
                                  w_results.size() /
                                  cc.get_waveguide_sample_rate());

    SRC_DATA sample_rate_info{
        w_results.data(),
        out_signal.data(),
        long(w_results.size()),
        long(out_signal.size()),
        0,
        0,
        0,
        cc.get_output_sample_rate() / double(cc.get_waveguide_sample_rate())};

    src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);
    return out_signal;
}
