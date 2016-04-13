#pragma once

#include "app_config.h"

#include <cereal/types/base_class.hpp>

namespace config {

class Waveguide : public virtual App {
public:
    virtual ~Waveguide() noexcept = default;

    float &get_oversample_ratio();
    float &get_filter_frequency();

    float get_oversample_ratio() const;
    float get_filter_frequency() const;

    float get_max_frequency() const;
    float get_waveguide_sample_rate() const;
    float get_divisions() const;

    template <typename Archive>
    void serialize(Archive &archive) {
        archive(cereal::virtual_base_class<App>(this),
                filter_frequency,
                oversample_ratio);
    }

private:
    float filter_frequency{500};
    float oversample_ratio{2};
};

}  // namespace config

std::vector<float> adjust_sampling_rate(std::vector<float> &w_results,
                                        const config::Waveguide &cc);
