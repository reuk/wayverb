#pragma once

#include "rayverb.h"

/// Describes the attenuation model that should be used to attenuate a raytrace.
/// There's probably a more elegant (runtime-polymorphic) way of doing this that
/// doesn't require both the HrtfConfig and the vector <Speaker> to be present
/// in the object at the same time.
struct AttenuationModel {
    enum Mode { SPEAKER, HRTF };
    Mode mode;
    Hrtf::Config hrtf;
    std::vector<Speaker> speakers;
};

template <>
struct JsonGetter<Speaker> {
    JsonGetter(Speaker& t)
            : t(t) {
    }

    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is a json object.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsObject();
    }

    /// Attempts to run a ConfigValidator on value.
    virtual void get(const rapidjson::Value& value) const {
        ConfigValidator cv;

        cv.addRequiredValidator("direction", t.direction);
        cv.addRequiredValidator("shape", t.coefficient);

        cv.run(value);
    }
    Speaker& t;
};

template <>
struct JsonGetter<HrtfConfig> {
    JsonGetter(HrtfConfig& t)
            : t(t) {
    }

    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is a json object.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsObject();
    }

    /// Attempts to run a ConfigValidator on value.
    virtual void get(const rapidjson::Value& value) const {
        ConfigValidator cv;

        cv.addRequiredValidator("facing", t.facing);
        cv.addRequiredValidator("up", t.up);

        cv.run(value);

        normalize(t.facing);
        normalize(t.up);
    }
    HrtfConfig& t;

private:
    static void normalize(cl_float3& v) {
        cl_float len =
            1.0 / sqrt(v.s[0] * v.s[0] + v.s[1] * v.s[1] + v.s[2] * v.s[2]);
        for (auto i = 0; i != sizeof(cl_float3) / sizeof(float); ++i) {
            v.s[i] *= len;
        }
    }
};

template <>
struct JsonGetter<AttenuationModel> {
    JsonGetter(AttenuationModel& t)
            : t(t)
            , keys({{AttenuationModel::SPEAKER, "speakers"},
                    {AttenuationModel::HRTF, "hrtf"}}) {
    }
    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is a json object containing just one valid key.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsObject() &&
               1 == std::count_if(keys.begin(),
                                  keys.end(),
                                  [&value](const auto& i) {
                                      return value.HasMember(i.second.c_str());
                                  });
    }

    /// Attempts to run a ConfigValidator on value.
    virtual void get(const rapidjson::Value& value) const {
        for (const auto& i : keys)
            if (value.HasMember(i.second.c_str()))
                t.mode = i.first;

        ConfigValidator cv;

        if (value.HasMember(keys.at(AttenuationModel::SPEAKER).c_str()))
            cv.addRequiredValidator(keys.at(AttenuationModel::SPEAKER).c_str(),
                                    t.speakers);

        if (value.HasMember(keys.at(AttenuationModel::HRTF).c_str()))
            cv.addRequiredValidator(keys.at(AttenuationModel::HRTF).c_str(),
                                    t.hrtf);

        cv.run(value);
    }
    AttenuationModel& t;
    std::map<AttenuationModel::Mode, std::string> keys;
};

