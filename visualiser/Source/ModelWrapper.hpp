#pragma once

#include "Collection.h"

#include "combined_config.h"
#include "common/scene_data.h"

namespace model {

template <>
class ValueWrapper<Vec3f> : public NestedValueWrapper<Vec3f> {
public:
    using NestedValueWrapper<Vec3f>::NestedValueWrapper;

    void set_value(const Vec3f& u, bool do_notify = true) override {
        x.set_value(u.x, do_notify);
        y.set_value(u.y, do_notify);
        z.set_value(u.z, do_notify);
    }

    void reseat(Vec3f& u) override {
        x.reseat(u.x);
        y.reseat(u.y);
        z.reseat(u.z);
    }

    ValueWrapper<float> x{this, t->x};
    ValueWrapper<float> y{this, t->y};
    ValueWrapper<float> z{this, t->z};
};

template <>
class ValueWrapper<config::Combined>
    : public NestedValueWrapper<config::Combined> {
public:
    using NestedValueWrapper<config::Combined>::NestedValueWrapper;

    void set_value(const config::Combined& u, bool do_notify = true) override {
        filter_frequency.set_value(u.filter_frequency, do_notify);
        oversample_ratio.set_value(u.oversample_ratio, do_notify);
        rays.set_value(u.rays, do_notify);
        impulses.set_value(u.impulses, do_notify);
        ray_hipass.set_value(u.ray_hipass, do_notify);
        do_normalize.set_value(u.do_normalize, do_notify);
        trim_predelay.set_value(u.trim_predelay, do_notify);
        trim_tail.set_value(u.trim_tail, do_notify);
        remove_direct.set_value(u.remove_direct, do_notify);
        volume_scale.set_value(u.volume_scale, do_notify);
        source.set_value(u.source, do_notify);
        mic.set_value(u.mic, do_notify);
        sample_rate.set_value(u.sample_rate, do_notify);
        bit_depth.set_value(u.bit_depth, do_notify);
    }

    void reseat(config::Combined& u) override {
        filter_frequency.reseat(u.filter_frequency);
        oversample_ratio.reseat(u.oversample_ratio);
        rays.reseat(u.rays);
        impulses.reseat(u.impulses);
        ray_hipass.reseat(u.ray_hipass);
        do_normalize.reseat(u.do_normalize);
        trim_predelay.reseat(u.trim_predelay);
        trim_tail.reseat(u.trim_tail);
        remove_direct.reseat(u.remove_direct);
        volume_scale.reseat(u.volume_scale);
        source.reseat(u.source);
        mic.reseat(u.mic);
        sample_rate.reseat(u.sample_rate);
        bit_depth.reseat(u.bit_depth);
    }

    ValueWrapper<float> filter_frequency{this, t->filter_frequency};
    ValueWrapper<float> oversample_ratio{this, t->oversample_ratio};
    ValueWrapper<int> rays{this, t->rays};
    ValueWrapper<int> impulses{this, t->impulses};
    ValueWrapper<float> ray_hipass{this, t->ray_hipass};
    ValueWrapper<bool> do_normalize{this, t->do_normalize};
    ValueWrapper<bool> trim_predelay{this, t->trim_predelay};
    ValueWrapper<bool> trim_tail{this, t->trim_tail};
    ValueWrapper<bool> remove_direct{this, t->remove_direct};
    ValueWrapper<float> volume_scale{this, t->volume_scale};
    ValueWrapper<Vec3f> source{this, t->source};
    ValueWrapper<Vec3f> mic{this, t->mic};
    ValueWrapper<float> sample_rate{this, t->sample_rate};
    ValueWrapper<int> bit_depth{this, t->bit_depth};
};

template <>
class ValueWrapper<VolumeType> : public NestedValueWrapper<VolumeType> {
public:
    using NestedValueWrapper<VolumeType>::NestedValueWrapper;

    void set_value(const VolumeType& u, bool do_notify = true) override {
        s0.set_value(u.s0, do_notify);
        s1.set_value(u.s1, do_notify);
        s2.set_value(u.s2, do_notify);
        s3.set_value(u.s3, do_notify);
        s4.set_value(u.s4, do_notify);
        s5.set_value(u.s5, do_notify);
        s6.set_value(u.s6, do_notify);
        s7.set_value(u.s7, do_notify);
    }

    void reseat(VolumeType& u) override {
        s0.reseat(u.s0);
        s1.reseat(u.s1);
        s2.reseat(u.s2);
        s3.reseat(u.s3);
        s4.reseat(u.s4);
        s5.reseat(u.s5);
        s6.reseat(u.s6);
        s7.reseat(u.s7);
    }

    ValueWrapper<float> s0{this, t->s[0]};
    ValueWrapper<float> s1{this, t->s[1]};
    ValueWrapper<float> s2{this, t->s[2]};
    ValueWrapper<float> s3{this, t->s[3]};
    ValueWrapper<float> s4{this, t->s[4]};
    ValueWrapper<float> s5{this, t->s[5]};
    ValueWrapper<float> s6{this, t->s[6]};
    ValueWrapper<float> s7{this, t->s[7]};
};

template <>
class ValueWrapper<Surface> : public NestedValueWrapper<Surface> {
public:
    using NestedValueWrapper<Surface>::NestedValueWrapper;

    void set_value(const Surface& u, bool do_notify = true) override {
        specular.set_value(u.specular, do_notify);
        diffuse.set_value(u.diffuse, do_notify);
    }

    void reseat(Surface& u) override {
        specular.reseat(u.specular);
        diffuse.reseat(u.diffuse);
    }

    ValueWrapper<VolumeType> specular{this, t->specular};
    ValueWrapper<VolumeType> diffuse{this, t->diffuse};
};

template <>
class ValueWrapper<SceneData::Material>
    : public NestedValueWrapper<SceneData::Material> {
public:
    using NestedValueWrapper<SceneData::Material>::NestedValueWrapper;

    void set_value(const SceneData::Material& u,
                   bool do_notify = true) override {
        name.set_value(u.name, do_notify);
        surface.set_value(u.surface, do_notify);
    }

    void reseat(SceneData::Material& u) override {
        name.reseat(u.name);
        surface.reseat(u.surface);
    }

    ValueWrapper<std::string> name{this, t->name};
    ValueWrapper<Surface> surface{this, t->surface};
};

template <>
class ValueWrapper<config::Microphone>
    : public NestedValueWrapper<config::Microphone> {
public:
    using NestedValueWrapper<config::Microphone>::NestedValueWrapper;

    void set_value(const config::Microphone& u,
                   bool do_notify = true) override {
        facing.set_value(u.facing, do_notify);
        shape.set_value(u.shape, do_notify);
    }

    void reseat(config::Microphone& u) override {
        facing.reseat(u.facing);
        shape.reseat(u.shape);
    }

    ValueWrapper<Vec3f> facing{this, t->facing};
    ValueWrapper<float> shape{this, t->shape};
};

template <>
class ValueWrapper<config::MicrophoneModel>
    : public NestedValueWrapper<config::MicrophoneModel> {
public:
    using NestedValueWrapper<config::MicrophoneModel>::NestedValueWrapper;

    void set_value(const config::MicrophoneModel& u,
                   bool do_notify = true) override {
        microphones.set_value(u.microphones, do_notify);
    }

    void reseat(config::MicrophoneModel& u) override {
        microphones.reseat(u.microphones);
    }

    ValueWrapper<std::vector<config::Microphone>> microphones{this,
                                                              t->microphones};
};

template <>
class ValueWrapper<config::HrtfModel>
    : public NestedValueWrapper<config::HrtfModel> {
public:
    using NestedValueWrapper<config::HrtfModel>::NestedValueWrapper;

    void set_value(const config::HrtfModel& u, bool do_notify = true) override {
        facing.set_value(u.facing, do_notify);
        up.set_value(u.up, do_notify);
    }

    void reseat(config::HrtfModel& u) override {
        facing.reseat(u.facing);
        up.reseat(u.up);
    }

    ValueWrapper<Vec3f> facing{this, t->facing};
    ValueWrapper<Vec3f> up{this, t->up};
};

//----------------------------------------------------------------------------//

class FullReceiverConfig {
public:
    config::AttenuationModel::Mode mode;
    config::MicrophoneModel microphone_model;
    config::HrtfModel hrtf_model;
};

template <>
class ValueWrapper<FullReceiverConfig>
    : public NestedValueWrapper<FullReceiverConfig> {
public:
    using NestedValueWrapper<FullReceiverConfig>::NestedValueWrapper;

    void set_value(const FullReceiverConfig& u,
                   bool do_notify = true) override {
        mode.set_value(u.mode, do_notify);
        microphone_model.set_value(u.microphone_model, do_notify);
        hrtf_model.set_value(u.hrtf_model, do_notify);
    }

    void reseat(FullReceiverConfig& u) override {
        mode.reseat(u.mode);
        microphone_model.reseat(u.microphone_model);
        hrtf_model.reseat(u.hrtf_model);
    }

    ValueWrapper<config::AttenuationModel::Mode> mode{this, t->mode};
    ValueWrapper<config::MicrophoneModel> microphone_model{this,
                                                           t->microphone_model};
    ValueWrapper<config::HrtfModel> hrtf_model{this, t->hrtf_model};
};

//----------------------------------------------------------------------------//

template <typename T>
class ValueWithWrapper : public ModelMember {
private:
    T t;
    ValueWrapper<T> wrapper{this, t};

public:
    ValueWithWrapper(ModelMember* owner, const T& t = T())
            : ModelMember(owner)
            , t(t) {
    }

    ValueWithWrapper(ModelMember* owner, T&& t)
            : ModelMember(owner)
            , t(std::move(t)) {
    }

    ValueWithWrapper(const ValueWithWrapper&) = delete;
    ValueWithWrapper& operator=(const ValueWithWrapper&) = delete;
    ValueWithWrapper(ValueWithWrapper&&) noexcept = delete;
    ValueWithWrapper& operator=(ValueWithWrapper&&) noexcept = delete;

    virtual ~ValueWithWrapper() noexcept = default;

    const T& get_value() const {
        return wrapper;
    }

    void set_value(const T& u, bool do_notify = true) {
        wrapper.set_value(u, do_notify);
    }

    const ValueWrapper<T>& get_wrapper() const {
        return wrapper;
    }

    ValueWrapper<T>& get_wrapper() {
        return wrapper;
    }
};

using Combined = ValueWithWrapper<config::Combined>;

}  // namespace model