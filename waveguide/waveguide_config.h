#pragma once

#include "app_config.h"

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

private:
    float filter_frequency{500};
    float oversample_ratio{2};
};

template <>
struct JsonGetter<Waveguide> {
    JsonGetter(Waveguide &t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    virtual bool check(const rapidjson::Value &value) const {
        JsonGetter<App> jg(t);
        return value.IsObject() && jg.check(value);
    }

    virtual void get(const rapidjson::Value &value) const {
        JsonGetter<App> jg(t);
        jg.get(value);

        ConfigValidator cv;

        cv.addOptionalValidator("waveguide_filter_frequency",
                                t.get_filter_frequency());
        cv.addOptionalValidator("waveguide_oversample_ratio",
                                t.get_oversample_ratio());

        cv.run(value);
    }

    Waveguide &t;
};
}

std::vector<float> adjust_sampling_rate(std::vector<float> &w_results,
                                        const config::Waveguide &cc);
