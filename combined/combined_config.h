#pragma once

#include "raytracer_config.h"
#include "waveguide_config.h"

namespace config {

class Combined : public Waveguide, public Raytracer {};

template <>
struct JsonGetter<Combined> {
    explicit JsonGetter(Combined& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    virtual bool check(const rapidjson::Value& value) const {
        JsonGetter<Raytracer> jg_r(t);
        JsonGetter<Waveguide> jg_w(t);
        return value.IsObject() && jg_r.check(value) && jg_r.check(value);
    }

    virtual void get(const rapidjson::Value& value) const {
        JsonGetter<Raytracer> jg_r(t);
        jg_r.get(value);

        JsonGetter<Waveguide> jg_w(t);
        jg_w.get(value);
    }

    Combined& t;
};

}  // namespace config
