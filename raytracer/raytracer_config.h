#pragma once

#include "app_config.h"

class RaytracerConfig : public virtual Config {
public:
    virtual ~RaytracerConfig() noexcept = default;

    /// Different components of the output impulse.
    enum class OutputMode {
        all,
        image,
        diffuse,
    };

    int& get_rays();
    int& get_impulses();
    float& get_ray_hipass();
    bool& get_do_normalize();
    bool& get_trim_predelay();
    bool& get_trim_tail();
    bool& get_remove_direct();
    float& get_volume_scale();

    int get_rays() const;
    int get_impulses() const;
    float get_ray_hipass() const;
    bool get_do_normalize() const;
    bool get_trim_predelay() const;
    bool get_trim_tail() const;
    bool get_remove_direct() const;
    float get_volume_scale() const;

private:
    int rays{1024 * 32};
    int impulses{64};
    float ray_hipass{45};
    bool do_normalize{true};
    bool trim_predelay{false};
    bool trim_tail{false};
    bool remove_direct{false};
    float volume_scale{1.0};
};

template <>
struct JsonGetter<RaytracerConfig> {
    JsonGetter(RaytracerConfig& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    virtual bool check(const rapidjson::Value& value) const {
        JsonGetter<Config> jg(t);
        return value.IsObject() && jg.check(value);
    }

    virtual void get(const rapidjson::Value& value) const {
        JsonGetter<Config> jg(t);
        jg.get(value);

        ConfigValidator cv;

        cv.addOptionalValidator("rays", t.get_rays());
        cv.addOptionalValidator("reflections", t.get_impulses());
        cv.addOptionalValidator("hipass", t.get_ray_hipass());
        cv.addOptionalValidator("normalize", t.get_do_normalize());
        cv.addOptionalValidator("volume_scale", t.get_volume_scale());
        cv.addOptionalValidator("trim_predelay", t.get_trim_predelay());
        cv.addOptionalValidator("trim_tail", t.get_trim_tail());
        cv.addOptionalValidator("remove_direct", t.get_remove_direct());

        cv.run(value);
    }

    RaytracerConfig& t;
};

/// JsonGetter for OutputMode is just a JsonEnumGetter with a specific map
template <>
struct JsonGetter<RaytracerConfig::OutputMode>
    : public JsonEnumGetter<RaytracerConfig::OutputMode> {
    JsonGetter(RaytracerConfig::OutputMode& t)
            : JsonEnumGetter(
                  t,
                  {{"all", RaytracerConfig::OutputMode::all},
                   {"image_only", RaytracerConfig::OutputMode::image},
                   {"diffuse_only", RaytracerConfig::OutputMode::diffuse}}) {
    }
    virtual ~JsonGetter() noexcept = default;
};
