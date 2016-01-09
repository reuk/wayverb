#pragma once

#include "config.h"

#define SPEED_OF_SOUND (340.0f)

class Config {
public:
    virtual ~Config() noexcept = default;

    Vec3f& get_source();
    Vec3f& get_mic();
    float& get_output_sample_rate();
    int& get_bit_depth();

    Vec3f get_source() const;
    Vec3f get_mic() const;
    float get_output_sample_rate() const;
    int get_bit_depth() const;

private:
    Vec3f source{0, 0, 0};
    Vec3f mic{0, 0, 0};
    float sample_rate{44100};
    int bit_depth{16};
};

template <>
struct JsonGetter<Config> {
    JsonGetter(Config& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    virtual bool check(const rapidjson::Value& value) const {
        return value.IsObject();
    }

    virtual void get(const rapidjson::Value& value) const {
        ConfigValidator cv;

        cv.addRequiredValidator("sample_rate", t.get_output_sample_rate());
        cv.addRequiredValidator("bit_depth", t.get_bit_depth());
        cv.addRequiredValidator("source_position", t.get_source());
        cv.addRequiredValidator("mic_position", t.get_mic());

        cv.run(value);
    }

    Config& t;
};
