#pragma once

#include "raytracer_config.h"
#include "waveguide_config.h"

class CombinedConfig : public WaveguideConfig, public RayverbConfig {};

template <>
struct JsonGetter<CombinedConfig> {
    JsonGetter(CombinedConfig& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    virtual bool check(const rapidjson::Value& value) const {
        JsonGetter<RayverbConfig> jg_r(t);
        JsonGetter<WaveguideConfig> jg_w(t);
        return value.IsObject() && jg_r.check(value) && jg_r.check(value);
    }

    virtual void get(const rapidjson::Value& value) const {
        JsonGetter<RayverbConfig> jg_r(t);
        jg_r.get(value);

        JsonGetter<WaveguideConfig> jg_w(t);
        jg_w.get(value);
    }

    CombinedConfig& t;
};

CombinedConfig read_config(const std::string & file);
